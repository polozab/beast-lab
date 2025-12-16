#include <Kokkos_Core.hpp>
#include <KokkosBlas3_gemm.hpp>
#include <iostream>
#include <cstdlib>

using scalar_t = double;
using HostViewA = Kokkos::View<scalar_t**, Kokkos::LayoutRight, Kokkos::HostSpace>;
using HostViewB = Kokkos::View<scalar_t**, Kokkos::LayoutRight, Kokkos::HostSpace>;
using HostViewC = Kokkos::View<scalar_t**, Kokkos::LayoutLeft, Kokkos::HostSpace>;
using DevViewA  = Kokkos::View<scalar_t**, Kokkos::LayoutRight>;
using DevViewB  = Kokkos::View<scalar_t**, Kokkos::LayoutRight>;
using DevViewC  = Kokkos::View<scalar_t**, Kokkos::LayoutLeft>;

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

void device_matmul(const DevViewA A, const DevViewB B, DevViewC C) {
    const double alpha = 1.0;
    const double beta  = 0.0;
    KokkosBlas::gemm("N", "N", alpha, A, B, beta, C);
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
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <matrix_size_N>\n";
            Kokkos::finalize();
            return 1;
        }

        size_t N = std::atoi(argv[1]);

        if (N == 0) {
            std::cerr << "Invalid size N.\n";
            Kokkos::finalize();
            return 1;
        }


        bool runCPU = false;
        if (argc > 2) runCPU = true;

        double total_flops = 2.0 * N * N * N;

        HostViewA h_A("A_host", N, N);
        HostViewB h_B("B_host", N, N);
        HostViewC h_C_ref("C_host_ref", N, N);
        HostViewC h_C_gpu("C_host_result", N, N);

        initialize_matrix(h_A);
        initialize_matrix(h_B);

        bool run_cpu = (argc >= 6);
        scalar_t cpu_sum = 0.0;

        std::cout << "[INFO] N:         " << N << "\n\n";
        if (run_cpu) std::cout << "[INFO] CPU ON\n\n";
        else std::cout << "[INFO] CPU OFF\n\n";

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
        
        DevViewA d_A("A_dev", N, N);
        DevViewB d_B("B_dev", N, N);
        DevViewC d_C("C_dev", N, N);

        Kokkos::deep_copy(d_A, h_A);
        Kokkos::deep_copy(d_B, h_B);

        Kokkos::Timer gpu_timer;
        device_matmul(d_A, d_B, d_C);
        double gpu_time = gpu_timer.seconds();
        double gpu_gflops = total_flops / (gpu_time * 1e9);

        Kokkos::deep_copy(h_C_gpu, d_C);
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
