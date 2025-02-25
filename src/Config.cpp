
#include "Config.hpp"
#include "Log.hpp"
#include <fstream>
#include <arpa/inet.h>

/**
 * @brief Removes trailing and leading spaces from a string
 */
std::string Config::trim(const std::string & str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos)
        return ""; // String is all whitespace
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

/**
 * @brief Loads (the config) file into a stringstream
 */
std::stringstream Config::load_file(const std::string & filename)
{
    std::ifstream       file(filename);
    std::stringstream   content;
    
    if (filename.substr(filename.find_last_of(".") + 1) != "conf")
        throw std::runtime_error("*.conf file extension required");
    if (!file.is_open())
        throw std::runtime_error("couldn't open config file");
    content << file.rdbuf();
    file.close();
    Log::log("Loaded config file:" + filename);
    return (content);
}

/**
 * @brief Parses the config file into a vector of server blocks
 */
void Config::parse_config(const std::string & filename, std::vector<ServerBlock> & server_blocks)
{
    std::string         line;
    std::stringstream   content;

    content = load_file(filename);
    while (getline(content, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#') // skip empty lines and comments
            continue ;
        else if (line == "server {")
        {
            ServerBlock server_block;
            parse_server_block(server_block, content, line);
            server_blocks.push_back(server_block);
            Log::log("New server block added");
            // if (line != "}") -> no need, checked in other function
                // throw std::runtime_error("in config file: missing closing bracket '}'");
        }
        else
            throw std::runtime_error("in config file: unexpected line");
    }
    // iterate through serverblocks and remove double entries (same host and port)
}

/**
 * @brief Parses one server {...} part of the config file
 */
void Config::parse_server_block(ServerBlock & block, std::stringstream & content, std::string & line)
{
    while (getline(content, line))
    {
        line = trim(line); // remove leading and trailing whitespaces
        if (line.empty() || line[0] == '#') // skip empty lines and comments
            continue ;

        std::string         directive, value;
        std::stringstream   ss(line);
        char delim = line.find_first_of(' \t');
        if (getline(ss, directive, delim) && getline(ss, value)) // doublecheck later for trailing whitespaces
        {
            if (directive == "listen")
            {
                // check value and amount (only 1 port allowed)
                block.port = std::atoi(value.c_str());
            }
            else if (directive == "host")
            {
                // check value and amount (only 1 host allowed)
                block.host = inet_addr(value.c_str());
            }
            else if (directive == "error_page")
            {
                // check value, split value into 2
                // check for double values (same error code)
                block.error_pages[std::atoi(value1.c_str())] = value2; //change this
            }
            else if (directive == "max_client_body_size")
            {
                // check value, check occurence
                block.max_client_body = std::atoi(value.c_str());
            }
            else if (directive == "location")
            {
                Location location;
                //parse_location(location, content, value);
                block.locations.push_back(location);
                Log::log("New location inside server block added");
            }
            else
                throw std::runtime_error("in sever block: unexpected line");
        }
        else if (line == "}") // end of server block
            return ;
        else
            throw std::runtime_error("in sever block: unexpected line");
    }
}

