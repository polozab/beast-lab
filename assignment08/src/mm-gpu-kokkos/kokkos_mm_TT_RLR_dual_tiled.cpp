#include <Kokkos_Core.hpp>
#include <Kokkos_DualView.hpp>
#include <iostream>
#include <cstdlib>

using scalar_t = double;
using ExecSpace = Kokkos::DefaultExecutionSpace;
using DualA = Kokkos::DualView<scalar_t**, Kokkos::LayoutRight, ExecSpace>;
using DualB = Kokkos::DualView<scalar_t**, Kokkos::LayoutLeft, ExecSpace>;
using DualC = Kokkos::DualView<scalar_t**, Kokkos::LayoutRight, ExecSpace>;
using HostViewA = DualA::t_host;
using HostViewB = DualB::t_host;
using HostViewC = DualC::t_host;
using DevViewA  = DualA::t_dev;
using DevViewB  = DualB::t_dev;
using DevViewC  = DualC::t_dev;

template <typename ViewType>
void initialize_matrix(ViewType A) {
    const auto rows = A.extent(0), cols = A.extent(1);
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

void device_matmul(const DevViewA A, const DevViewB B, DevViewC C, size_t N,
                   int tile_m, int tile_n, int threads_per_team, int vec_len) {
    using ES = Kokkos::DefaultExecutionSpace;
    const int tiles_m = (N + tile_m - 1) / tile_m;
    const int tiles_n = (N + tile_n - 1) / tile_n;
    const int league_size = tiles_m * tiles_n;

    auto policy = vec_len < 0
        ? Kokkos::TeamPolicy<ES>(league_size, threads_per_team, Kokkos::AUTO)
        : Kokkos::TeamPolicy<ES>(league_size, threads_per_team, vec_len);

    Kokkos::parallel_for("TeamMatMulTiledRLR", policy, KOKKOS_LAMBDA(const auto& team) {
        const int tile_id = team.league_rank();
        const int bi = tile_id / tiles_n;
        const int bj = tile_id - bi * tiles_n;
        const int i0 = bi * tile_m;
        const int j0 = bj * tile_n;

        Kokkos::parallel_for(Kokkos::TeamThreadRange(team, tile_m), [&](int ii) {
            const int i = i0 + ii;
            if (i >= (int)N) return;
            Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, tile_n), [&](int jj) {
                const int j = j0 + jj;
                if (j >= (int)N) return;
                scalar_t sum = 0.0;
                for (int k = 0; k < (int)N; ++k) {
                    sum += A(i, k) * B(k, j);
                }
                C(i, j) = sum;
            });
        });
    });
    Kokkos::fence();
}

template <typename ViewType>
scalar_t checksum(const ViewType& mat) {
    scalar_t sum = 0.0;
    for (size_t i = 0; i < mat.extent(0); ++i)
        for (size_t j = 0; j < mat.extent(1); ++j)
            sum += mat(i, j);
    return sum;
}

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        if (argc < 6) {
            std::cerr << "Usage: " << argv[0]
                      << " <matrix_size_N> <tile_m> <tile_n> <threads_per_team> <vec len (-1 = AUTO)> <opt.: set 1 for CPU>\n";
            Kokkos::finalize();
            return 1;
        }

        const size_t N            = std::atoi(argv[1]);
        const int tile_m          = std::atoi(argv[2]);
        const int tile_n          = std::atoi(argv[3]);
        const int threads_per_team= std::atoi(argv[4]);
        const int vec_len         = std::atoi(argv[5]);
        const bool run_cpu        = argc >= 7;

        if (N == 0 || tile_m <= 0 || tile_n <= 0) {
            std::cerr << "Invalid size or tile.\n";
            Kokkos::finalize();
            return 1;
        }

        const double total_flops = 2.0 * N * N * N;

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

        scalar_t cpu_sum = 0.0;
        if (run_cpu) {
            Kokkos::Timer cpu_timer;
            cpu_matmul(h_A, h_B, h_C_ref, N);
            double cpu_time   = cpu_timer.seconds();
            double cpu_gflops = total_flops / (cpu_time * 1e9);
            cpu_sum           = checksum(h_C_ref);

            std::cout << "[CPU] Time:       " << cpu_time   << " s\n";
            std::cout << "[CPU] GFLOP/s:    " << cpu_gflops << "\n";
            std::cout << "[CPU] Checksum:   " << cpu_sum    << "\n";
        }

        dv_A.sync<ExecSpace>();
        dv_B.sync<ExecSpace>();

        DevViewA d_A = dv_A.d_view;
        DevViewB d_B = dv_B.d_view;
        DevViewC d_C = dv_C.d_view;

        Kokkos::Timer gpu_timer;
        device_matmul(d_A, d_B, d_C, N, tile_m, tile_n, threads_per_team, vec_len);
        double gpu_time   = gpu_timer.seconds();
        double gpu_gflops = total_flops / (gpu_time * 1e9);

        dv_C.modify<ExecSpace>();
        dv_C.sync<Kokkos::HostSpace>();
        scalar_t gpu_sum = checksum(h_C_gpu);

        std::cout << "[GPU] Time:       " << gpu_time   << " s\n";
        std::cout << "[GPU] GFLOP/s:    " << gpu_gflops << "\n";
        std::cout << "[GPU] Checksum:   " << gpu_sum    << "\n";

        if (run_cpu) {
            double rel_err = std::abs(cpu_sum - gpu_sum) / (std::abs(cpu_sum) + 1e-10);
            std::cout << "[CHECK] Relative Error: " << rel_err << "\n";
            std::cout << (rel_err < 1e-8 ? "[PASS] Results match.\n" : "[FAIL] Results differ.\n");
        }
    }
    Kokkos::finalize();
    return 0;
}
