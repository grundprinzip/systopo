#include <systopo.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <sys/types.h>
#include <dirent.h>

#include <stdexcept>

using namespace systopo;

bool is_size(std::string data)
{
    if (data.find("K") != std::string::npos || data.find("M") != std::string::npos)
        return true;
    else
        return false;
}

size_t parse_size(std::string data)
{
    std::stringstream ss;
    size_t result;
    
    // First check which size, caches should be K or M but not G :)
    if (size_t found = data.find("K") != std::string::npos)
    {
        ss << data.substr(0, found);
        ss >> result;
        return result * 1024;
        
    } else if (size_t found = data.find("M") != std::string::npos)
    {
        ss << data.substr(0, found);
        ss >> result;
        return result * 1024 * 1024;
        
    } else {
        throw std::runtime_error("Cannot parse this: " + data);
    }
}


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
    
    // Check if its a number
    if (is_size(list))
    {
        result.push_back(parse_size(list));
        return result;
    }

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

std::vector<std::string> getFolders(std::string path)
{

    DIR *dp;
    struct dirent *dirp;

    std::vector<std::string> result;
    

     if((dp = opendir(path.c_str())) == NULL)
     {
         std::cout << "Failed to open " << path << std::endl;
         exit(2);
     }

     while ((dirp = readdir(dp)) != NULL)
     {
         // Only directories and not . and ..
         if (dirp->d_type == DT_DIR && dirp->d_name[0] != '.')
         {
             result.push_back(std::string(dirp->d_name));
         }
     }

     closedir(dp);
     
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

#define CACHE_ATTR_SIZE 6
std::string cache_attrs[CACHE_ATTR_SIZE] = {"coherency_line_size", "level",
                                            "number_of_sets", "physical_line_partition",
                                            "size", "ways_of_associativity"};

Cache parse_cache_data(const std::string p)
{
    Cache result;

    result.coherency_line_size = GET_LIST_FOR_PATH(p + "/" + cache_attrs[0])[0];
    result.level = GET_LIST_FOR_PATH(p + "/" + cache_attrs[1])[0];
    result.number_of_sets = GET_LIST_FOR_PATH(p + "/" + cache_attrs[2])[0];
    result.physical_line_partition = GET_LIST_FOR_PATH(p + "/" + cache_attrs[3])[0];
    result.size = GET_LIST_FOR_PATH(p + "/" + cache_attrs[4])[0];
    result.ways_of_associativity = GET_LIST_FOR_PATH(p + "/" + cache_attrs[5])[0];

    // Get the type
    std::stringstream s;
    s << p << "/type";
    result.type = get_file_contents(s.str());
    
    return result;
}


/*
 * Read the system topology of cores and caches
 *
 * We can get the number of online CPUs from the online value inf
 * sys. Which is then parsed and used as a hook into which CPUs can be
 * further parsed.
 */
System systopo::getSystemTopology()
{
    System result;

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
        std::string cpath = cache_path.str();

        std::vector<std::string> caches_f = getFolders(cache_path.str());
        for(size_t j=0; j < caches_f.size(); ++j)
        {
            std::stringstream tmp;
            tmp << cpath << caches_f[j];
            cpu.caches.push_back(parse_cache_data(tmp.str()));
        }

        result.cpus.push_back(cpu);
        
    }
    

    return result;
}
