
#include "Config.hpp"
#include <fstream>
#include <arpa/inet.h>

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos)
        return ""; // String is all whitespace
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

/* Function for parsing any type of config file */
//note: same host and port names
void Config::parse_config(std::string filename, std::vector<ServerBlock> & server_blocks)
{
    std::ifstream   file(filename);
    std::string     line;
    
    //put it first into a stringstream -> and close it, then parse
    if (filename.substr(filename.find_last_of(".") + 1) != "conf")
        throw std::runtime_error("*.conf file extension required");
    if (!file.is_open())
        throw std::runtime_error("couldn't open config file");
    while (getline(file, line))
    {
        line = trim(line);
        if (line.empty())
            continue ;
        else if (line == "server {")
        {
            ServerBlock server_block;
            parse_server_block(server_block, file, line);
            server_blocks.push_back(server_block);
            if (line != "}")
                throw std::runtime_error("in config file: missing closing bracket '}'");
        }
        else
            throw std::runtime_error("in config file: unexpected line");
    }
    file.close();
}

void parse_server_block(ServerBlock & block, std::ifstream &file, std::string & line)
{
    
    while (getline(file, line))
    {
        line = trim(line); // remove leading and trailing whitespaces
        if (line.empty()) // skip empty lines
            continue ;
        else if (line == "}") // end of server block
            return ;

        std::string         directive, value;
        std::stringstream   ss(line);
        char delim = line.find_first_of(' \t');
        if (getline(ss, directive, delim) && getline(ss, value)) // doublecheck later for trailing whitespaces
        {
            if (directive == "listen")
            {
                // check value
                block.port = std::stoi(value);
            }
            else if (directive == "server_name")
            {
                // check value
                block.host = inet_addr(value.c_str());
            }
            else if (directive == "error_page")
            {
                // check value, split value
                block.error_pages.push_back(std::make_pair(404,"error/404.html")); //change this
            }
            else if (directive == "max_client_body")
            {
                // check value
                block.max_client_body = std::stoi(value);
            }
            else if (directive == "location")
            {
                Location location;
                //parse_location(location, file, value);
                block.locations.push_back(location);
            }
            else
                throw std::runtime_error("in sever block: unexpected line");
        }
        else
            throw std::runtime_error("in sever block: unexpected line");
    }
}

