#include <Kokkos_Core.hpp>
#include <cuda_runtime.h>
#include <iostream>

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        int device_id = Kokkos::Cuda().cuda_device();
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, device_id);

        std::cout << "CUDA Device Properties:\n";
        std::cout << "Device Name: " << prop.name << "\n";
        std::cout << "Total Global Memory: " << prop.totalGlobalMem << " bytes\n";
        std::cout << "Shared Memory Per Block: " << prop.sharedMemPerBlock << " bytes\n";
        std::cout << "Max Threads Per Block: " << prop.maxThreadsPerBlock << "\n";
        std::cout << "Multiprocessor Count: " << prop.multiProcessorCount << "\n";
        std::cout << "Compute Capability: " << prop.major << "." << prop.minor << "\n";
    }
    Kokkos::finalize();
    return 0;
}
