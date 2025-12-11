#include <Kokkos_Core.hpp>
#include <hip/hip_runtime.h>
#include <iostream>

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        int device_id = Kokkos::Experimental::HIP().hip_device();
        hipDeviceProp_t prop;
        hipGetDeviceProperties(&prop, device_id);

        std::cout << "HIP Device Properties:\n";
        std::cout << "Device Name: " << prop.name << "\n";
        std::cout << "Total Global Memory: " << prop.totalGlobalMem << " bytes\n";
        std::cout << "Shared Memory Per Block: " << prop.sharedMemPerBlock << " bytes\n";
        std::cout << "Max Threads Per Block: " << prop.maxThreadsPerBlock << "\n";
        std::cout << "Multiprocessor Count: " << prop.multiProcessorCount << "\n";
        std::cout << "GFX IP Version: " << prop.gcnArchName << "\n";
    }
    Kokkos::finalize();
    return 0;
}
