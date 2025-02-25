
#include "Config.hpp"

/**
 * @brief Removes trailing and leading spaces from a string, as well as trailing comments;
 * replaces consecutive spaces with a single space.
 */
std::string Config::trim(const std::string & str)
{
    std::string result = str;

    // replace tabs with spaces
    std::replace(result.begin(), result.end(), '\t', ' ');

    // remove trailing comment
    size_t comment = result.find("#");
    if (comment != std::string::npos)
        result = result.substr(0, comment);

    // remove spaces
    size_t first = result.find_first_not_of(" ");
    if (first == std::string::npos)
        return ""; // string is all whitespace
    size_t last = result.find_last_not_of(" ");
    return (result.substr(first, (last - first + 1)));

    // replace consecutive spaces with single space
    std::string cleaned;
    bool consecutive_spaces = false;
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] == ' ') {
            if (!consecutive_spaces) {
                cleaned += ' ';
                consecutive_spaces = true;
            }
        } else {
            cleaned += result[i];
            consecutive_spaces = false;
        }
    }
    return (cleaned);
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
        line = trim(line); // remove abundant whitespaces and comments
        if (line.empty()) // skip empty lines
            continue ;

        if (line == "}") // end of server block
            return ;
        else if (line[line.size() - 1] == ';' 
            || (line.find("location") != std::string::npos && line[line.size() - 1] == '{')) // remove ;
            line = line.substr(0, line.size() - 1);
        else
            throw std::runtime_error("in server block: missing semicolon: " + line);
        parse_server_block_directives(line, block, content);
    }
    // / iterate through locations and remove double entries (same name or root?)
}

int Config::has_only_digits(char *str)
{
    while (str && *str)
    {
        if (!std::isdigit(*str++))
            return (0);
    }
    return (1);
}

void Config::parse_server_block_directives(std::string & line, ServerBlock & block, std::stringstream & content)
{
    std::string         directive, value;
    std::stringstream   ss(line);
    if (getline(ss, directive, ' ') && getline(ss, value))
    {
        if (directive == "listen")
        {
            if (block.port != -1)
                throw std::runtime_error("server cannot have multiple ports");
            // optional: add more checks for valid ports
            if (value.size() > 5 || !has_only_digits(const_cast<char *>(value.c_str())))
                throw std::runtime_error("invalid port number" + value);
            block.port = std::atoi(value.c_str());
        }
        else if (directive == "host")
        {
            if (block.host != inet_addr("255.255.255.255"))
                throw std::runtime_error("host already declared");
            block.host = inet_addr(value.c_str());
            // optional: add more checks for valid ip addresses
            if (block.host == inet_addr("255.255.255.255"))
                throw std::runtime_error("invalid IP address");
        }
        else if (directive == "error_page")
        {
            std::string code, path;
            std::stringstream ss(value);
            if (!getline(ss, code, ' ') || !getline(ss, path) || path.find(' ') != std::string::npos)
                throw std::runtime_error("in server block: error_page directive requires 2 arguments");
            if (code.size() != 3 || !has_only_digits(const_cast<char *>(code.c_str())))
                throw std::runtime_error("invalid error code " + code);
            // optional: check if path is a valid syntax and valid path
            if (block.error_pages.find(std::atoi(code.c_str())) != block.error_pages.end())
                throw std::runtime_error("error page already exists for " + code);
            block.error_pages[std::atoi(code.c_str())] = path;
        }
        else if (directive == "client_max_body_size")
        {
            if (block.max_client_body != 0 || !has_only_digits(const_cast<char *>(value.c_str())))
                throw std::runtime_error("max body size already declared or invalid size");
            block.max_client_body = std::atol(value.c_str());
            // optional: check if size is too big or too small, check for overflow, change type
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

