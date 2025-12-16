#include <Kokkos_Core.hpp>
#include<KokkosBlas3_gemm.hpp>
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

void device_matmul(const DevView2D A, const DevView2D B, DevView2D C, size_t N) {
    const double alpha = 1.0;
    const double beta  = 0.0;
    KokkosBlas::gemm("N", "N", alpha, A, B, beta, C);
}

scalar_t checksum(const HostView2D& mat) {
    scalar_t sum = 0.0;
    for (size_t i = 0; i < mat.extent(0); ++i)
        for (size_t j = 0; j < mat.extent(1); ++j)
            sum += mat(i, j);
    return sum;
}

struct bench_result
{
    double cpu_time;
    double cpu_gflops;
    double gpu_time;
    double gpu_gflops;
};


void bench(size_t N, bench_result* res) {
    Kokkos::initialize(N, res);
    {

        double total_flops = 2.0 * N * N * N;

        // Allocate host matrices
        HostView2D h_A("A_host", N, N);
        HostView2D h_B("B_host", N, N);
        HostView2D h_C_ref("C_host_ref", N, N);
        HostView2D h_C_gpu("C_host_result", N, N);

        initialize_matrix(h_A);
        initialize_matrix(h_B);

        // --- CPU Kokkos matmul ---
        Kokkos::Timer cpu_timer;
        cpu_matmul(h_A, h_B, h_C_ref, N);
        double cpu_time = cpu_timer.seconds();
        double cpu_gflops = total_flops / (cpu_time * 1e9);
        scalar_t cpu_sum = checksum(h_C_ref);

        std::cerr << "[CPU] Time:       " << cpu_time << " s\n";
        std::cerr << "[CPU] GFLOP/s:    " << cpu_gflops << "\n";
        std::cerr << "[CPU] Checksum:   " << cpu_sum << "\n";

        // --- Device matmul ---
        DevView2D d_A("A_dev", N, N);
        DevView2D d_B("B_dev", N, N);
        DevView2D d_C("C_dev", N, N);

        Kokkos::deep_copy(d_A, h_A);
        Kokkos::deep_copy(d_B, h_B);

        Kokkos::Timer gpu_timer;
        device_matmul(d_A, d_B, d_C, N);
        double gpu_time = gpu_timer.seconds();
        double gpu_gflops = total_flops / (gpu_time * 1e9);

        Kokkos::deep_copy(h_C_gpu, d_C);
        scalar_t gpu_sum = checksum(h_C_gpu);

        std::cerr << "[GPU] Time:       " << gpu_time << " s\n";
        std::cerr << "[GPU] GFLOP/s:    " << gpu_gflops << "\n";
        std::cerr << "[GPU] Checksum:   " << gpu_sum << "\n";

        // --- Result Comparison ---
        double rel_err = std::abs(cpu_sum - gpu_sum) / (std::abs(cpu_sum) + 1e-10);
        std::cerr << "[CHECK] Relative Error: " << rel_err << "\n";
        std::cerr << (rel_err < 1e-8 ? "[PASS] Results match.\n" : "[FAIL] Results differ.\n");

        // Return results
        res->cpu_time = cpu_time;
        res->cpu_gflops = cpu_gflops;
        res->gpu_time = gpu_time;
        res->gpu_gflops = gpu_gflops;
    }
    Kokkos::finalize();
    return;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <matrix_size_N>\n";
        return 1;
    }

    size_t N_max = std::atoi(argv[1]);
    if (N_max == 0) {
        std::cerr << "Invalid size N.\n";
        return 1;
    }

    std::cout << "N, CPU_Time, GPU_Time, CPU_FLOPS, GPU_FLOPS" << "\n";

    for (size_t N = 200; N < N_max; N += 200)
    {
        bench_result *res = new bench_result;
        bench(N, res)
        
    }
    
    

}
