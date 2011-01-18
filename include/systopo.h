// The contents of this library is based on
// http://www.mjmwired.net/kernel/Documentation/cputopology.txt

#include <vector>
#include <string>

namespace systopo
{

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

    /*
     * Based on the information provided by sysfs this is the entry
     * method to the library that reads the contents from there.
     */
    System * getSystemTopology();

}
