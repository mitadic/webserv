
#include "Config.hpp"

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
    Log::log("loaded config file: " + filename);
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
        if (line.empty()) // skip empty lines
            continue ;
        if (line == "server {")
        {
            Log::log("new server block");
            ServerBlock server_block;
            parse_server_block(server_block, content, line);
            server_blocks.push_back(server_block);
        }
        else
            throw std::runtime_error("in config file: unexpected line");
    }
    validate_blocks(server_blocks);
    // iterate through serverblocks and remove double entries (same host and port)
}

/**
 * @brief Loops through server blocks and checks for missing directives,
 * for same host-port combinations and for same prefixes in location blocks
 */
void    Config::validate_blocks(std::vector<ServerBlock> & server_blocks)
{
    Log::log("Before validation:");
    Log::log(server_blocks);
    
    if (server_blocks.empty())
        throw std::runtime_error("Empty vector of server blocks");

    std::vector<ServerBlock>::iterator server_it;
    std::map<int, in_addr_t> host_port_combo;
    for (server_it = server_blocks.begin(); server_it != server_blocks.end(); ++server_it)
    {
        if (server_it->port == -1 || server_it->host == ft_inet("255.255.255.255") || server_it->max_client_body == 0)
            throw std::runtime_error("missing directive 'listen', 'host' or 'client_max_body_size' inside server block");
        if (host_port_combo.find(server_it->port) != host_port_combo.end())
        {
            if (host_port_combo[server_it->port] == server_it->host)
            {
                Log::log("same host-port combination, removing server block...");
                server_it = server_blocks.erase(server_it); // remove server block from blocks
                server_it--; // adjust iterator
            }
        }
        else 
            host_port_combo[server_it->port] = server_it->host;
        if (server_it->locations.empty())
            continue ;
        // optional: throw std::runtime_error("missing location block inside server block");
        std::sort(server_it->locations.begin(), server_it->locations.end(), compare_prefix); // sort by prefix: longest -> shortest
        std::vector<Location>::iterator location_it, tmp;
        for (location_it = server_it->locations.begin(); location_it != server_it->locations.end(); ++location_it)
        {
            if ((location_it + 1) != server_it->locations.end() && same_prefix(*location_it, *(location_it + 1)))  // error for same prefixes
                throw std::runtime_error("locations with same prefix cannot override each other");

            // optional: check if root / exists
            // optional: check if location is empty?
        }
    }
}

/**
 * @brief Parses one server {...} part of the config file into a server block
 */
void Config::parse_server_block(ServerBlock & block, std::stringstream & content, std::string & line)
{
    while (getline(content, line))
    {
        line = trim(line); // remove abundant whitespaces and comments
        if (line.empty()) // skip empty lines
            continue ;

        if (line == "}") // end of server block
            return ;
        else if (line[line.size() - 1] == ';' 
            || (line.find("location") != std::string::npos && line[line.size() - 1] == '{')) // remove ';'
        {
            line = line.substr(0, line.size() - 1);
            line = trim(line);
        }   
        else
            throw std::runtime_error("in server block: missing semicolon: " + line);
        parse_server_block_directives(line, block, content);
    }
}

/**
 * @brief Parses the directives inside a server block e.g. listen, error_page
 */
void Config::parse_server_block_directives(std::string & line, ServerBlock & block, std::stringstream & content)
{
    std::string         directive, value;
    std::stringstream   ss(line);
    if (getline(ss, directive, ' ') && getline(ss, value))
    {
        if (directive == "listen")
            parse_port(block, value);
        else if (directive == "host")
            parse_host(block, value);
        else if (directive == "error_page")
            parse_error_page(block, value);
        else if (directive == "client_max_body_size")
            parse_client_body(block, value);
        else if (directive == "location")
        {
            Log::log("new location inside server block");
            Location location;
            parse_location(value, location, content);
            block.locations.push_back(location);
        }
        else
            throw std::runtime_error("in sever block: unknown directive: " + directive);
    }
    else
        throw std::runtime_error("in sever block: unexpected line: " + line);
}

/**
 * @brief Parses the location block inside a server block
 */
void Config::parse_location(std::string & line, Location & block, std::stringstream & content)
{
    block.location = check_location_prefix(block, line);
    int directive_count = 0;
    while (getline(content, line))
    {
        line = trim(line); // remove unnecessary whitespaces
        if (line.empty()) // skip empty lines
            continue ;

        if (line == "}" && directive_count == 0)
            throw std::runtime_error("empty location block");
        if (line == "}") // end of block
            return ;
        else if (line[line.size() - 1] == ';') // check if ';' is there and remove it from the line
        {
            line = line.substr(0, line.size() - 1);
            line = trim(line);
        } 
        else
            throw std::runtime_error("in location block: missing semicolon: " + line);
        parse_location_block_directives(line, block, content);
        directive_count++;
    }
}

/**
 * @brief Parses the directives inside a location block e.g. root, index
 */
void Config::parse_location_block_directives(std::string & line, Location & block, std::stringstream & content)
{
    std::string         directive, value;
    std::stringstream   ss(line);
    if (getline(ss, directive, ' ') && getline(ss, value))
    {
        if (directive == "root")
            parse_root(block, value); 
        else if (directive == "index")
            parse_index(block, value);
        else if (directive == "upload")
            parse_upload(block, value);
        else if (directive == "allowed_methods")
            parse_allowed_methods(block, value);
        else if (directive == "autoindex")
            parse_autoindex(block, value);
        else if (directive == "cgi_extension")
            parse_cgi_extension(block, value);
        else if (directive == "return")
            parse_redirect(block, value);
        else
            throw std::runtime_error("in location block: unknown directive: " + directive);
    }
    else
        throw std::runtime_error("in location block: unexpected line: " + line);
}











