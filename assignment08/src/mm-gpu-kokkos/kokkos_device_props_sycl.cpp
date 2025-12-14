#include <Kokkos_Core.hpp>
#include <CL/sycl.hpp>  // SYCL header

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        // Access SYCL device used by Kokkos
        sycl::device dev = Kokkos::Experimental::SYCL().sycl_queue().get_device();

        std::cout << "Device Name: " << dev.get_info<sycl::info::device::name>() << "\n";
        std::cout << "Vendor: " << dev.get_info<sycl::info::device::vendor>() << "\n";

        std::count << "\n";

        std::cout << "Max Work Group Size: " << dev.get_info<sycl::info::device::max_work_group_size>() << "\n";
        std::cout << "Max Work Group Size per dimension (2 dimensions): " << dev.get_info<sycl::info::device::max_work_item_sizes<2>>() << "\n";
        std::cout << "Max Number Of Dimensions: " << dev.get_info<sycl::info::device::max_work_item_dimensions>() << "\n";
        std::cout << "Max Compute Units: " << dev.get_info<sycl::info::device::max_compute_units>() << "\n";
        std::cout << "Local Memory: " << dev.get_info<sycl::info::device::local_mem_size>() << " bytes\n";
        
        std::count << "\n";

        std::cout << "Preferred Vector Width (double): " << dev.get_info<sycl::info::device::preferred_vector_width_double>() << "\n";
        //std::cout << "Max Sub Work Group Size: " << dev.get_info<sycl::info::device::sub_group_sizes>() << "\n";
        std::cout << "Max Number Of Sub Work Groups: " << dev.get_info<sycl::info::device::max_num_sub_groups>() << "\n";

    }
    Kokkos::finalize();
}
