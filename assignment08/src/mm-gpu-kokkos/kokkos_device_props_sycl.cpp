#include <Kokkos_Core.hpp>
#include <CL/sycl.hpp>  // SYCL header

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        // Access SYCL device used by Kokkos
        sycl::device dev = Kokkos::Experimental::SYCL().sycl_queue().get_device();

        std::cout << "Device Name: " << dev.get_info<sycl::info::device::name>() << "\n";
        std::cout << "Vendor: " << dev.get_info<sycl::info::device::vendor>() << "\n";
        std::cout << "Max Compute Units: " << dev.get_info<sycl::info::device::max_compute_units>() << "\n";
        std::cout << "Local Memory: " << dev.get_info<sycl::info::device::local_mem_size>() << " bytes\n";
        std::cout << "Max Work Group Size: " << dev.get_info<sycl::info::device::max_work_group_size>() << "\n";
    }
    Kokkos::finalize();
}
