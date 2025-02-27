
#include "Config.hpp"

void    Config::parse_redirect(Location & block, std::string & value)
{
    if (block.redirect.second.empty() == false)
        throw std::runtime_error("double declaration of 'return'");
    std::string code, url;
    std::stringstream ss(value);
    if (!getline(ss, code, ' ') || !getline(ss, url) || (url.find(' ') != std::string::npos))
        throw std::runtime_error("in location block: return directive requires 2 arguments");
    if (block.location == url)
        throw std::runtime_error("directive redirects to itself"); // this could create an infinite loop
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
    if (value != "index.html")
        throw std::runtime_error("expected 'index.html'");
    // optional: allow index.htm besides index.html
    block.index = value;
}

void    Config::parse_root(Location & block, std::string & value)
{
    check_valid_path(value, ROOT);
    if (block.root.empty() == false)
        throw std::runtime_error("double occurrence of 'root'");
    block.root = value;
}

void    Config::parse_upload(Location & block, std::string & value)
{
    if (block.upload_location.empty() == false)
        throw std::runtime_error("double occurrence of 'upload'");
    check_valid_path(value, ROOT);
    block.upload_allowed = true;
    block.upload_location = value;
}

void    Config::parse_cgi_extension(Location & block, std::string & value)
{
    if (block.cgi_extensions.empty() == false)
        throw std::runtime_error("double declaration of 'cgi_extension'");
        
    char str[value.size() + 1];
    std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    while (token)
    {
        // optional: implement checks if extension is accepted
        std::string extension = token;
            block.cgi_extensions.push_back(extension);
        token = std::strtok(nullptr, " ");
    }
}

void    Config::parse_allowed_methods(Location & block, std::string & value)
{
    if (block.get == true || block.post == true || block.del == true)
        throw std::runtime_error("double occurrence of 'allowed_methods'");

    char str[value.size() + 1];
    std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    while (token)
    {
        if (std::strcmp(token, "GET") == 0 && block.get == false)
            block.get = true;
        else if (std::strcmp(token, "POST") == 0 && block.post == false)
            block.post = true;
        else if (std::strcmp(token, "DELETE") == 0 && block.del == false)
            block.del = true;
        else
            throw std::runtime_error("Unknown or duplicate method in location block");
        token = std::strtok(nullptr, " ");
    }
}

std::string    Config::check_location_prefix(Location & block, std::string & value)
{ 
    std::string name;

    char str[value.size() + 1];
    std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    name = token;
    if (!token)
        throw std::runtime_error("Location: missing location name");
    token = std::strtok(nullptr, " ");
    if (token)
        throw std::runtime_error("Location: too many arguments");
    check_valid_path(name, LOCATION);
    return (name);
}