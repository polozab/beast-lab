#include <Kokkos_Core.hpp>
#include <Kokkos_DualView.hpp>
#include <iostream>
#include <cstdlib>

using scalar_t = double;
using ExecSpace = Kokkos::DefaultExecutionSpace;
using DualA = Kokkos::DualView<scalar_t**, Kokkos::LayoutRight, ExecSpace>;
using DualB = Kokkos::DualView<scalar_t**, Kokkos::LayoutRight, ExecSpace>;
using DualC = Kokkos::DualView<scalar_t**, Kokkos::LayoutLeft, ExecSpace>;
using HostViewA = DualA::t_host;
using HostViewB = DualB::t_host;
using HostViewC = DualC::t_host;
using DevViewA  = DualA::t_dev;
using DevViewB  = DualB::t_dev;
using DevViewC  = DualC::t_dev;

template <typename ViewType>
void initialize_matrix(ViewType A) {
    auto rows = A.extent(0), cols = A.extent(1);
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
            A(i, j) = static_cast<scalar_t>((i * cols + j) % 100 + 1);
}

void cpu_matmul(const HostViewA A, const HostViewB B, HostViewC C, size_t N) {
    using HostExecSpace = Kokkos::DefaultHostExecutionSpace;
    Kokkos::parallel_for(
        "CPUMatMul",
        Kokkos::MDRangePolicy<HostExecSpace, Kokkos::Rank<2>>({0, 0}, {N, N}),
        KOKKOS_LAMBDA(int i, int j) {
            scalar_t sum = 0.0;
            for (size_t k = 0; k < N; ++k)
                sum += A(i, k) * B(k, j);
            C(i, j) = sum;
        });
    Kokkos::fence();
}

void device_matmul(const DevViewA A, const DevViewB B, DevViewC C, size_t N, int teams, int threads_per_team, int vec_len) {
    using ExecSpace = Kokkos::DefaultExecutionSpace;
    const int rows_per_team = (N + teams - 1) / teams;

    auto policy = vec_len < 0
        ? Kokkos::TeamPolicy<ExecSpace>(teams, threads_per_team, Kokkos::AUTO)
        : Kokkos::TeamPolicy<ExecSpace>(teams, threads_per_team, vec_len);

    Kokkos::parallel_for("TeamMatMul", policy, KOKKOS_LAMBDA(const auto& team) {
        const int t = team.league_rank();
        const int row_start = t * rows_per_team;
        const int row_end   = row_start + rows_per_team < (int)N ? row_start + rows_per_team : (int)N;

        for (int i = row_start; i < row_end; ++i) {
            Kokkos::parallel_for(Kokkos::TeamThreadRange(team, N), [=](int j) {
                scalar_t sum = 0.0;
                Kokkos::parallel_reduce(
                    Kokkos::ThreadVectorRange(team, N),
                    [&](int k, scalar_t& lsum){ lsum += A(i,k)*B(k,j); }, sum);
                C(i,j) = sum;
            });
        }
    });
    Kokkos::fence();
}

scalar_t checksum(const HostViewC& mat) {
    scalar_t sum = 0.0;
    for (size_t i = 0; i < mat.extent(0); ++i)
        for (size_t j = 0; j < mat.extent(1); ++j)
            sum += mat(i, j);
    return sum;
}

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        if (argc < 5) {
            std::cerr << "Usage: " << argv[0] << " <matrix_size_N> <teams> <threads_per_team> <vec len (-1 = AUTO)> <opt.: set 1 for CPU>\n";
            Kokkos::finalize();
            return 1;
        }

        size_t N = std::atoi(argv[1]);
        int teams = std::atoi(argv[2]);
        int threads_per_team = std::atoi(argv[3]);
        int vec_len = std::atoi(argv[4]);

        if (N == 0) {
            std::cerr << "Invalid size N.\n";
            Kokkos::finalize();
            return 1;
        }

        double total_flops = 2.0 * N * N * N;

        DualA dv_A("A", N, N);
        DualB dv_B("B", N, N);
        DualC dv_C("C", N, N);

        HostViewA h_A = dv_A.h_view;
        HostViewB h_B = dv_B.h_view;
        HostViewC h_C_ref = dv_C.h_view;
        HostViewC h_C_gpu = dv_C.h_view;

        initialize_matrix(h_A);
        initialize_matrix(h_B);
        dv_A.modify<Kokkos::HostSpace>();
        dv_B.modify<Kokkos::HostSpace>();

        bool run_cpu = (argc >= 6);
        scalar_t cpu_sum = 0.0;

        if (run_cpu) {
            Kokkos::Timer cpu_timer;
            cpu_matmul(h_A, h_B, h_C_ref, N);
            double cpu_time = cpu_timer.seconds();
            double cpu_gflops = total_flops / (cpu_time * 1e9);
            cpu_sum = checksum(h_C_ref);

            std::cout << "[CPU] Time:       " << cpu_time << " s\n";
            std::cout << "[CPU] GFLOP/s:    " << cpu_gflops << "\n";
            std::cout << "[CPU] Checksum:   " << cpu_sum << "\n";
        }

        dv_A.sync<Kokkos::DefaultExecutionSpace>();
        dv_B.sync<Kokkos::DefaultExecutionSpace>();

        DevViewA d_A = dv_A.d_view;
        DevViewB d_B = dv_B.d_view;
        DevViewC d_C = dv_C.d_view;

        Kokkos::Timer gpu_timer;
        device_matmul(d_A, d_B, d_C, N, teams, threads_per_team, vec_len);
        double gpu_time = gpu_timer.seconds();
        double gpu_gflops = total_flops / (gpu_time * 1e9);

        dv_C.modify<Kokkos::DefaultExecutionSpace>();
        dv_C.sync<Kokkos::HostSpace>();
        scalar_t gpu_sum = checksum(h_C_gpu);

        std::cout << "[GPU] Time:       " << gpu_time << " s\n";
        std::cout << "[GPU] GFLOP/s:    " << gpu_gflops << "\n";
        std::cout << "[GPU] Checksum:   " << gpu_sum << "\n";

        if (run_cpu) {
            double rel_err = std::abs(cpu_sum - gpu_sum) / (std::abs(cpu_sum) + 1e-10);
            std::cout << "[CHECK] Relative Error: " << rel_err << "\n";
            std::cout << (rel_err < 1e-8 ? "[PASS] Results match.\n" : "[FAIL] Results differ.\n");
        }
    }
    Kokkos::finalize();
    return 0;
}

