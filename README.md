# systopo by Martin Grund <grundprinzip@gmail.com>

`systopo` is a very simple parser written in C++ that allows to parse the content of `/sys/devices/system/cpu` and automatically creates a small hierarchy of dependent objects. The goal of `systopo` is to automatically retrieve information about the NUMA nodes, number of cores and the cache hierarchy of the system.

To build `systopo` download the archive and executed `make`. This will build the library and a small test binary. 

NOTE: The program will only run on recent Linux systems with a 2.6.* kernel and probably only Intel processors.

The system topology features the following fields

	struct Cache
    {
        size_t coherency_line_size;
        size_t level;
        size_t number_of_sets;
        size_t physical_line_partition;
        size_t size;
        size_t ways_of_associativity;
    
        std::string type;
    
    };

    struct Topology
    {
        size_t core_id;
        size_t physical_package_id;
        std::vector<size_t> core_siblings;
        std::vector<size_t> thread_siblings;
    };
    
    struct CPU
    {
        std::vector<Cache> caches;
        Topology topology;
    };


    struct System
    {
        std::vector<CPU> cpus;
        std::vector<size_t> online_cpus;
        std::vector<size_t> offline_cpus;
    };