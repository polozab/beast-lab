#include <Kokkos_Core.hpp>
#include <KokkosBlas3_gemm.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>

using scalar_t = double;
using HostViewA = Kokkos::View<scalar_t**, Kokkos::LayoutRight, Kokkos::HostSpace>;
using HostViewB = Kokkos::View<scalar_t**, Kokkos::LayoutLeft, Kokkos::HostSpace>;
using HostViewC = Kokkos::View<scalar_t**, Kokkos::LayoutRight, Kokkos::HostSpace>;
using DevViewA  = Kokkos::View<scalar_t**, Kokkos::LayoutRight>;
using DevViewB  = Kokkos::View<scalar_t**, Kokkos::LayoutLeft>;
using DevViewC  = Kokkos::View<scalar_t**, Kokkos::LayoutRight>;

template <typename ViewType>
void initialize_matrix(ViewType A) {
    using HostExecSpace = Kokkos::DefaultHostExecutionSpace;
    const int rows = static_cast<int>(A.extent(0));
    const int cols = static_cast<int>(A.extent(1));
    Kokkos::parallel_for(
        "InitMatrix",
        Kokkos::MDRangePolicy<HostExecSpace, Kokkos::Rank<2>>({0, 0}, {rows, cols}),
        KOKKOS_LAMBDA(int i, int j) {
            A(i, j) = static_cast<scalar_t>((i * cols + j) % 100 + 1);
        });
    Kokkos::fence();
}

void handwritten_matmul(const DevViewA A, const DevViewB B, DevViewC C, size_t N,
                   int tile_m, int tile_n, int threads_per_team, int vec_len) {
    using ExecSpace = Kokkos::DefaultExecutionSpace;
    const int tiles_m = (N + tile_m - 1) / tile_m;
    const int tiles_n = (N + tile_n - 1) / tile_n;
    const int league_size = tiles_m * tiles_n;

    auto policy = vec_len < 0
        ? Kokkos::TeamPolicy<ExecSpace>(league_size, threads_per_team, Kokkos::AUTO)
        : Kokkos::TeamPolicy<ExecSpace>(league_size, threads_per_team, vec_len);

    Kokkos::parallel_for("TeamMatMulTiled", policy, KOKKOS_LAMBDA(const auto& team) {
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
                    sum += A(i,k)*B(k,j);
                }
                C(i,j) = sum;
            });
        });
    });
    Kokkos::fence();
}


void kernel_matmul(const DevViewA A, const DevViewB B, DevViewC C) {
    const double alpha = 1.0;
    const double beta  = 0.0;
    KokkosBlas::gemm("N", "N", alpha, A, B, beta, C);
}

scalar_t checksum(const HostViewC& mat) {
    using HostExecSpace = Kokkos::DefaultHostExecutionSpace;
    const int rows = static_cast<int>(mat.extent(0));
    const int cols = static_cast<int>(mat.extent(1));
    scalar_t sum = 0.0;
    Kokkos::parallel_reduce(
        "Checksum",
        Kokkos::MDRangePolicy<HostExecSpace, Kokkos::Rank<2>>({0, 0}, {rows, cols}),
        KOKKOS_LAMBDA(int i, int j, scalar_t& lsum) { lsum += mat(i, j); },
        sum);
    return sum;
}

struct ErrorMetrics {
    scalar_t max_abs;
    double l2_diff;
    double rel_l2;
};

ErrorMetrics compute_error_metrics(const HostViewC& ref, const HostViewC& test) {
    using HostExecSpace = Kokkos::DefaultHostExecutionSpace;
    const int rows = static_cast<int>(ref.extent(0));
    const int cols = static_cast<int>(ref.extent(1));
    auto policy = Kokkos::MDRangePolicy<HostExecSpace, Kokkos::Rank<2>>({0, 0}, {rows, cols});

    scalar_t max_abs = 0.0;
    Kokkos::parallel_reduce(
        "MaxAbsError",
        policy,
        KOKKOS_LAMBDA(int i, int j, scalar_t& lmax) {
            scalar_t diff = ref(i, j) - test(i, j);
            if (diff < 0) diff = -diff;
            if (diff > lmax) lmax = diff;
        },
        Kokkos::Max<scalar_t>(max_abs));

    double diff2 = 0.0;
    Kokkos::parallel_reduce(
        "L2Diff",
        policy,
        KOKKOS_LAMBDA(int i, int j, double& lsum) {
            const double diff = static_cast<double>(ref(i, j) - test(i, j));
            lsum += diff * diff;
        },
        diff2);

    double ref2 = 0.0;
    Kokkos::parallel_reduce(
        "L2Ref",
        policy,
        KOKKOS_LAMBDA(int i, int j, double& lsum) {
            const double val = static_cast<double>(ref(i, j));
            lsum += val * val;
        },
        ref2);

    ErrorMetrics metrics;
    metrics.max_abs = max_abs;
    metrics.l2_diff = std::sqrt(diff2);
    metrics.rel_l2 = metrics.l2_diff / (std::sqrt(ref2) + 1e-12);
    return metrics;
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


        bool run_cpu = (argc > 2);
        double total_flops = 2.0 * N * N * N;

        HostViewA h_A("A_host", N, N);
        HostViewB h_B("B_host", N, N);
        HostViewC h_C_ref("C_host_ref", N, N);
        HostViewC h_C_gpu("C_host_result", N, N);

        initialize_matrix(h_A);
        initialize_matrix(h_B);
        
        std::cout << "[INFO] N:         " << N << "\n";
        
        scalar_t hw_sum = 0.0;
            
        Kokkos::Timer hw_timer;
        handwritten_matmul(h_A, h_B, h_C_ref, N, 64, 16, 64, 16);
        double hw_time = hw_timer.seconds();
        double hw_gflops = total_flops / (hw_time * 1e9);
        hw_sum = checksum(h_C_ref);
        
        std::cout << "[Handwritten] Time:       " << hw_time << " s\n";
        std::cout << "[Handwritten] GFLOP/s:    " << hw_gflops << "\n";
        std::cout << "[Handwritten] Checksum:   " << hw_sum << "\n";
        
        DevViewA d_A("A_dev", N, N);
        DevViewB d_B("B_dev", N, N);
        DevViewC d_C("C_dev", N, N);

        Kokkos::deep_copy(d_A, h_A);
        Kokkos::deep_copy(d_B, h_B);

        Kokkos::Timer gpu_timer;
        kernel_matmul(d_A, d_B, d_C);
        double gpu_time = gpu_timer.seconds();
        double gpu_gflops = total_flops / (gpu_time * 1e9);

        Kokkos::deep_copy(h_C_gpu, d_C);
        scalar_t gpu_sum = checksum(h_C_gpu);
        ErrorMetrics err = compute_error_metrics(h_C_ref, h_C_gpu);

        std::cout << "[Kernel] Time:       " << gpu_time << " s\n";
        std::cout << "[Kernel] GFLOP/s:    " << gpu_gflops << "\n";
        std::cout << "[Kernel] Checksum:   " << gpu_sum << "\n\n";

        double rel_err = std::abs(hw_sum - gpu_sum) / (std::abs(hw_sum) + 1e-10);
        std::cout << "[CHECK] Relative Error: " << rel_err << "\n";
        std::cout << "[CHECK] Max Abs Error:  " << err.max_abs << "\n";
        std::cout << "[CHECK] L2 Error:       " << err.l2_diff << "\n";
        std::cout << "[CHECK] Rel L2 Error:   " << err.rel_l2 << "\n";
        std::cout << (rel_err < 1e-8 ? "[PASS] Results match.\n" : "[FAIL] Results differ.\n");

    }
    Kokkos::finalize();
    return 0;
}
