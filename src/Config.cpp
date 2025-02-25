
#include "Config.hpp"

/**
 * @brief Removes trailing and leading spaces from a string, as well as trailing comments
 */
std::string Config::trim(const std::string & str)
{
    std::string result = str;

    size_t comment = result.find("#"); // remove trailing comment
    if (comment != std::string::npos)
        result = result.substr(0, comment);

    size_t first = result.find_first_not_of(" \t"); // remove spaces
    if (first == std::string::npos)
        return ""; // String is all whitespace
    size_t last = result.find_last_not_of(" \t");
    return (result.substr(first, (last - first + 1)));
}

/**
 * @brief Replaces tabs in a string with spaces
 */
std::string Config::replace_tabs_with_spaces(std::string & line)
{
    std::string result = line;
    std::replace(result.begin(), result.end(), '\t', ' ');
    return (result);
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
    Log::log("Loaded config file: " + filename);
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
        line = replace_tabs_with_spaces(line);
        line = trim(line);
        if (line.empty()) // skip empty lines
            continue ;
        if (line == "server {")
        {
            ServerBlock server_block;
            parse_server_block(server_block, content, line);
            server_blocks.push_back(server_block);
        }
        else
            throw std::runtime_error("in config file: unexpected line");
    }
    // iterate through serverblocks and remove double entries (same host and port)
}

/**
 * @brief Parses one server {...} part of the config file into a server block
 */
void Config::parse_server_block(ServerBlock & block, std::stringstream & content, std::string & line)
{
    while (getline(content, line))
    {
        line = replace_tabs_with_spaces(line);
        line = trim(line); // remove leading and trailing whitespaces and comments
        if (line.empty()) // skip empty lines
            continue ;

        if (line == "}") // end of server block
            return ;
        else if (line[line.size() - 1] == ';') // check if ';' is there and remove it from the line
            line = line.substr(0, line.size() - 1);
        else if (line.find("location") != std::string::npos && line[line.size() - 1] == '{') // remove } from location line
            line = line.substr(0, line.size() - 1);
        else
            throw std::runtime_error("in server block: missing semicolon: " + line);
        parse_server_block_directives(line, block, content);
    }
    // / iterate through locations and remove double entries (same name or root?)
}

void Config::parse_server_block_directives(std::string & line, ServerBlock & block, std::stringstream & content)
{
    std::string         directive, value;
    std::stringstream   ss(line);
    if (getline(ss, directive, ' ') && getline(ss, value))
    {
        value = trim(value);
        if (directive == "listen")
        {
            // check value and amount (only 1 port allowed)
            block.port = std::atoi(value.c_str());
        }
        else if (directive == "host")
        {
            // check value and amount (only 1 host allowed)
            block.host = inet_addr(value.c_str());
            // check if host is valid
        }
        else if (directive == "error_page")
        {
            // check for double values (same error code)
            // check if error code is valid
            // ? check if page exists
            std::string value1, value2;
            std::stringstream ss(value);
            if (!getline(ss, value1, ' ') || !getline(ss, value2))
                throw std::runtime_error("in server block: error_page directive requires 2 arguments");
            value2 = trim(value2);
            if (value2.empty() || value2.find(' ') != std::string::npos)
                throw std::runtime_error("in server block: error_page directive requires 2 arguments");
            block.error_pages[std::atoi(value1.c_str())] = value2; 
        }
        else if (directive == "client_max_body_size")
        {
            // check value, check occurence
            // check if size is too big or too small?
            block.max_client_body = std::atoi(value.c_str());
        }
        else if (directive == "location")
        {
            Location location;
            parse_location(value, location, content);
            block.locations.push_back(location);
            Log::log("New location inside server block added");
        }
        else
            throw std::runtime_error("in sever block: unknown directive: " + directive);
    }
    else
        throw std::runtime_error("in sever block: unexpected line: " + line);
}

void Config::parse_location(std::string & line, Location & block, std::stringstream & content)
{
    block.location = line;
    // check if location is valid

    while (getline(content, line))
    {
        line = replace_tabs_with_spaces(line);
        line = trim(line); // remove leading and trailing whitespaces
        if (line.empty() || line[0] == '#') // skip empty lines and comments
            continue ;

        if (line == "}") // end of block
            return ;
        else if (line[line.size() - 1] == ';') // check if ';' is there and remove it from the line
            line = line.substr(0, line.size() - 1);
        else
            throw std::runtime_error("in location block: missing semicolon: " + line);
        parse_location_block_directives(line, block, content);
    }
}

void Config::parse_location_block_directives(std::string & line, Location & block, std::stringstream & content)
{
    std::string         directive, value;
    std::stringstream   ss(line);
    if (getline(ss, directive, ' ') && getline(ss, value))
    {
        value = trim(value);
        if (directive == "root")
        {
            // check for double declaration
            // check if valid, if contains spaces etc.
            block.root = value;
        }
        else if (directive == "index")
        {
            // check for double declaration
            // check if valid, if contains spaces etc.
            block.index = value;
        }
        else if (directive == "upload")
        {
            // check for double declaration
            // check if valid, if contains spaces etc.
            block.upload_allowed = true;
            block.upload_location = value;
        }
        else if (directive == "allowed_methods")
        {
            // check for double declaration
            // parse methods with strtok
            if (value.find("GET") != std::string::npos) //hardcoded solution tb removed
                block.get = true;
            if (value.find("POST") != std::string::npos)
                block.post = true;
            if (value.find("DELETE") != std::string::npos)
                block.del = true;
        }
        else if (directive == "autoindex")
        {
            // check for double declaration
            if (value == "on")
                block.autoindex = true;
            else if (value != "off")
                throw std::runtime_error("in location block: autoindex has to be on or off");
        }
        else if (directive == "cgi_extension")
        {
            // check for double declaration
            // parse extensions with strtok
            std::string extension = ".py"; // hardcoded solution tb removed
            block.cgi_extensions.push_back(extension);
        }
        else if (directive == "return")
        {
            // check for double declaration
            std::string value1, value2;
            std::stringstream ss(value);
            if (!getline(ss, value1, ' ') || !getline(ss, value2))
                throw std::runtime_error("in location block: return directive requires 2 arguments");
            value2 = trim(value2);
            if (value2.empty() || value2.find(' ') != std::string::npos)
                throw std::runtime_error("in location block: return directive requires 2 arguments");
            block.redirect = std::make_pair(std::atoi(value1.c_str()), value2);
        }
        else
            throw std::runtime_error("in location block: unknown directive: " + directive);
    }
    else
        throw std::runtime_error("in location block: unexpected line: " + line);
}

