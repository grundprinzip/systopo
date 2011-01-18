#include <systopo.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace systopo;

/*
 * Parsing a given list from sysfs
 *
 * The human readable list can be in the format of: 0,1,2,3 or 0-3 or
 * 0,2-3,5 so all those formats have to be supported. The rules are
 * quite easy: First split by comma, than parse if it's a range or a
 * single value, if its a range parse and add it.
 */
std::vector<size_t> parse_list(std::string list)
{
    size_t tmp;
    std::vector<size_t> result;
    
    std::stringstream ss(list);
    std::string part;

    while (std::getline(ss, part, ','))
    {
        // Check if part contains a -
        size_t pos = part.find('-');
        if (pos != std::string::npos)
        {
            std::stringstream left(part.substr(0, pos));
            std::stringstream right(part.substr(pos, part.size()));

            size_t l,r;
            left >> l;
            right >> r;

            for(size_t i=l; i < r; ++i)
                result.push_back(i);

            
        } else {

            std::stringstream ps(part);
            ps >> tmp;
            result.push_back(tmp);
        }
    }

    return result;
}

std::string get_file_contents(std::string path)
{
    std::ifstream f(path.c_str());
    std::ostringstream tmp;

    tmp << f.rdbuf();

    f.close();
        
    return tmp.str();
}

#define SYSTOPO_ONLINE_PATH "/sys/devices/system/cpu/online"
#define SYSTOPO_BASE_PATH "/sys/devices/system/cpu/cpu"
#define GET_LIST_FOR_PATH(p) parse_list(get_file_contents(p))

/*
 * Read the system topology of cores and caches
 *
 * We can get the number of online CPUs from the online value inf
 * sys. Which is then parsed and used as a hook into which CPUs can be
 * further parsed.
 */
System* systopo::getSystemTopology()
{
    System* result = new System;

    // Get online CPUs
    std::vector<size_t> cpus = parse_list(get_file_contents(SYSTOPO_ONLINE_PATH));

    for(size_t i=0; i < cpus.size(); ++i)
    {
        CPU cpu;
        
        // core_id, core_siblings_list, physical_package_id, thread_siblings_list
        Topology topology;

        {
            std::stringstream top_path;
            top_path << SYSTOPO_BASE_PATH << i << "/topology/core_id";
            topology.core_id = GET_LIST_FOR_PATH(top_path.str())[0];
        }

        {
            std::stringstream top_path;
            top_path << SYSTOPO_BASE_PATH << i << "/topology/core_siblings_list";
            topology.core_siblings = GET_LIST_FOR_PATH(top_path.str());
        }

        {
            std::stringstream top_path;
            top_path << SYSTOPO_BASE_PATH << i << "/topology/thread_siblings_list";
            topology.thread_siblings = GET_LIST_FOR_PATH(top_path.str());
        }

        {
            std::stringstream top_path;
            top_path << SYSTOPO_BASE_PATH << i << "/topology/physical_package_id";
            topology.physical_package_id = GET_LIST_FOR_PATH(top_path.str())[0];
        }

        cpu.topology = topology;
        
        std::stringstream cache_path;
        cache_path << SYSTOPO_BASE_PATH << i << "/cache/";


        result->cpus.push_back(cpu);
        
    }
    

    return result;
}
