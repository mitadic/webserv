
#include "Config.hpp"
#include <fstream>

void Config::parse_config(std::string filename, std::vector<ServerBlock> & server_blocks)
{
    std::ifstream   file(filename);
    std::string     line;

    if (!file.is_open())
        throw std::runtime_error("couldn't open config file");
    while (getline(file, line))
    {
        if (line.empty())
            continue ;
        else if (line == "server {")
        {
            ServerBlock block;
            //parse block;
            server_blocks.push_back(block);
            if (line != "}")
                throw std::runtime_error("in config file: missing closing braxcket '}'");
        }
        else
            throw std::runtime_error("in config file: unexpected line");
    }
    file.close();
}
