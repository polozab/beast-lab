#include <Kokkos_Core.hpp>
#include <iostream>
#include <cmath>
#include <cstdlib>

using scalar_t = double;
using HostView2D = Kokkos::View<scalar_t**, Kokkos::LayoutRight, Kokkos::HostSpace>;
using DevView2D  = Kokkos::View<scalar_t**, Kokkos::LayoutRight>;

void initialize_matrix(HostView2D A) {
    auto rows = A.extent(0), cols = A.extent(1);
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
            A(i, j) = static_cast<scalar_t>((i * cols + j) % 100 + 1);
}

void cpu_matmul(const HostView2D A, const HostView2D B, HostView2D C, size_t N) {
	using HostExecSpace = Kokkos::DefaultHostExecutionSpace;
    Kokkos::parallel_for("CPUMatMul",
        Kokkos::MDRangePolicy<HostExecSpace, Kokkos::Rank<2>>({0, 0}, {N, N}),
        KOKKOS_LAMBDA(int i, int j) {
            scalar_t sum = 0.0;
            for (size_t k = 0; k < N; ++k)
                sum += A(i, k) * B(k, j);
            C(i, j) = sum;
        });
    Kokkos::fence();
}

void device_matmul(const DevView2D A, const DevView2D B, DevView2D C, size_t N, int AmountOfTeams, int threads_per_team) {
    using ExecSpace = Kokkos::DefaultExecutionSpace;
auto vec_len   = Kokkos::AUTO;

auto policy = Kokkos::TeamPolicy<ExecSpace>(AmountOfTeams, threads_per_team, vec_len);

Kokkos::parallel_for("TeamMatMul", policy, KOKKOS_LAMBDA(const auto& team) {
  const int i = team.league_rank(); // row
  Kokkos::parallel_for(Kokkos::TeamThreadRange(team, N), [=](int j) {
    scalar_t sum = 0.0;
    Kokkos::parallel_reduce(Kokkos::ThreadVectorRange(team, N),
      [&](int k, scalar_t& lsum){ lsum += A(i,k)*B(k,j); }, sum);
    C(i,j) = sum;
  });
});
Kokkos::fence();

}

scalar_t checksum(const HostView2D& mat) {
    scalar_t sum = 0.0;
    for (size_t i = 0; i < mat.extent(0); ++i)
        for (size_t j = 0; j < mat.extent(1); ++j)
            sum += mat(i, j);
    return sum;
}

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " <matrix_size_N> <AmountOfTeams> <threads_per_team> <opt.: set 1 for CPU>\n";

            Kokkos::finalize();
            return 1;
        }

        size_t N = std::atoi(argv[1]);
        int AmountOfTeams = std::atoi(argv[2]);
        int threads_per_team = std::atoi(argv[3]);
        if (N == 0) {
            std::cerr << "Invalid size N.\n";
            Kokkos::finalize();
            return 1;
        }

        double total_flops = 2.0 * N * N * N;

        // Allocate host matrices
        HostView2D h_A("A_host", N, N);
        HostView2D h_B("B_host", N, N);
        HostView2D h_C_ref("C_host_ref", N, N);
        HostView2D h_C_gpu("C_host_result", N, N);

        initialize_matrix(h_A);
        initialize_matrix(h_B);


        // --- CPU Kokkos matmul ---
        scalar_t cpu_sum
        if (argc == 5) {
        Kokkos::Timer cpu_timer;
        cpu_matmul(h_A, h_B, h_C_ref, N);
        double cpu_time = cpu_timer.seconds();
        double cpu_gflops = total_flops / (cpu_time * 1e9);
        cpu_sum = checksum(h_C_ref);

        std::cout << "[CPU] Time:       " << cpu_time << " s\n";
        std::cout << "[CPU] GFLOP/s:    " << cpu_gflops << "\n";
        std::cout << "[CPU] Checksum:   " << cpu_sum << "\n";
        }
        // --- Device matmul ---
        DevView2D d_A("A_dev", N, N);
        DevView2D d_B("B_dev", N, N);
        DevView2D d_C("C_dev", N, N);

        Kokkos::deep_copy(d_A, h_A);
        Kokkos::deep_copy(d_B, h_B);

        Kokkos::Timer gpu_timer;
        device_matmul(d_A, d_B, d_C, N, AmountOfTeams, threads_per_team);
        double gpu_time = gpu_timer.seconds();
        double gpu_gflops = total_flops / (gpu_time * 1e9);

        Kokkos::deep_copy(h_C_gpu, d_C);
        scalar_t gpu_sum = checksum(h_C_gpu);

        std::cout << "[GPU] Time:       " << gpu_time << " s\n";
        std::cout << "[GPU] GFLOP/s:    " << gpu_gflops << "\n";
        std::cout << "[GPU] Checksum:   " << gpu_sum << "\n";

        // --- Result Comparison ---
        if (argc == 5) {
        double rel_err = std::abs(cpu_sum - gpu_sum) / (std::abs(cpu_sum) + 1e-10);
        std::cout << "[CHECK] Relative Error: " << rel_err << "\n";
        std::cout << (rel_err < 1e-8 ? "[PASS] Results match.\n" : "[FAIL] Results differ.\n");
        }
    }
    Kokkos::finalize();
    return 0;
}
