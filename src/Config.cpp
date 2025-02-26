
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
    validate_blocks(server_blocks);
    // iterate through serverblocks and remove double entries (same host and port)
}

/**
 * @brief Loops through server blocks and checks for missing directives
 */
void    Config::validate_blocks(std::vector<ServerBlock> & server_blocks)
{
    std::vector<ServerBlock>::iterator server;
    for (server = server_blocks.begin(); server != server_blocks.end(); ++server)
    {
        if (server->port == -1 || server->host == inet_addr("255.255.255.255") || server->max_client_body == 0)
            throw std::runtime_error("missing directive 'listen', 'host' or 'client_max_body_size' inside server block");
    // check for same hosts and ports
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
    // iterate through locations and remove double entries (same name or root?)
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
            if (block.max_client_body != 0 || !has_only_digits(const_cast<char *>(value.c_str()))
                || (std::strtod(value.c_str(), NULL) > UINT32_MAX))
                throw std::runtime_error("max body size already declared or invalid size");
            block.max_client_body = std::atol(value.c_str());
            // optional: change type or size
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
    if (line.find(' ') != std::string::npos)
        throw std::runtime_error("invalid path");
    // optional: check if location path and syntax are valid
    block.location = line;

    while (getline(content, line))
    {
        line = trim(line); // remove unnecessary whitespaces
        if (line.empty()) // skip empty lines
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

void    Config::parse_allowed_methods(Location & block, std::string & value)
{
    if (block.get == true || block.post == true || block.del == true)
        throw std::runtime_error("double occurrence of 'allowed_methods'");

    char *str = std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    while (token)
    {
        if (std::strcmp(token, "GET") == 0)
            block.get = true;
        else if (std::strcmp(token, "POST") == 0)
            block.post = true;
        else if (std::strcmp(token, "DELETE") == 0)
            block.del = true;
        else
            throw std::runtime_error("Unknown method in location block");
        token = std::strtok(nullptr, " ");
    }
}

void    Config::parse_cgi_extension(Location & block, std::string & value)
{
    if (block.cgi_extensions.empty() == false)
        throw std::runtime_error("double declaration of 'cgi_extension'");
        
    char *str = std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    while (token)
    {
        // optional: implement checks if extension is accepted
        std::string extension = token;
            block.cgi_extensions.push_back(extension);
        token = std::strtok(nullptr, " ");
    }
}

void    Config::parse_redirect(Location & block, std::string & value)
{
    if (block.redirect.second.empty() == false)
        throw std::runtime_error("double declaration of 'return'");
    std::string code, url;
    std::stringstream ss(value);
    if (!getline(ss, code, ' ') || !getline(ss, url) || (url.find(' ') != std::string::npos))
        throw std::runtime_error("in location block: return directive requires 2 arguments");
    block.redirect = std::make_pair(std::atoi(code.c_str()), url);
}

void    Config::parse_autoindex(Location & block, std::string & value)
{
    if (block.autoindex == true)
        throw std::runtime_error("double declaration of 'autoindex'");
    if (value == "on")
        block.autoindex = true;
    else if (value != "off")
        throw std::runtime_error("in location block: autoindex requires value 'on' or 'off'");
}

void    Config::parse_index(Location & block, std::string & value)
{
    if (block.index.empty() == false)
        throw std::runtime_error("double occurrence of 'index'");
    // check if valid, if contains spaces etc.
    block.index = value;
}


void    Config::parse_root(Location & block, std::string & value)
{
    // check if valid, if contains spaces etc.
    if (block.root.empty() == false)
        throw std::runtime_error("double occurrence of 'root'");
    block.root = value;
}

void    Config::parse_upload(Location & block, std::string & value)
{
    if (block.upload_location.empty() == false)
        throw std::runtime_error("double occurrence of 'upload'");
    // check if path is valid or contains spaces etc.
    block.upload_allowed = true;
    block.upload_location = value;
}

