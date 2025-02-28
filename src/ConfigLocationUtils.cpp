
#include "Config.hpp"

void    Config::parse_redirect(Location & block, std::string & value)
{
    if (block.get_redirect().second.empty() == false)
        throw std::runtime_error("double declaration of 'return'");
    std::string code, url;
    std::stringstream ss(value);
    if (!getline(ss, code, ' ') || !getline(ss, url) || (url.find(' ') != std::string::npos))
        throw std::runtime_error("in location block: return directive requires 2 arguments");
    if (block.get_path() == url)
        throw std::runtime_error("directive redirects to itself"); // this could create an infinite loop
    block.set_redirect(std::atoi(code.c_str()), url);
}

void    Config::parse_autoindex(Location & block, std::string & value)
{
    if (block.is_autoindex() == true)
        throw std::runtime_error("double declaration of 'autoindex'");
    if (value == "on")
        block.set_autoindex(true);
    else if (value != "off")
        throw std::runtime_error("in location block: autoindex requires value 'on' or 'off'");
}

void    Config::parse_index(Location & block, std::string & value)
{
    if (block.get_index().empty() == false)
        throw std::runtime_error("double occurrence of 'index'");
    if (value != "index.html")
        throw std::runtime_error("expected 'index.html'");
    // optional: allow index.htm besides index.html
    block.set_index(value);
}

void    Config::parse_root(Location & block, std::string & value)
{
    check_valid_path(value, ROOT);
    if (block.get_root().empty() == false)
        throw std::runtime_error("double occurrence of 'root'");
    block.set_root(value);
}

void    Config::parse_upload(Location & block, std::string & value)
{
    if (block.get_upload_location().empty() == false)
        throw std::runtime_error("double occurrence of 'upload'");
    check_valid_path(value, ROOT);
    block.set_upload_allowed(true);
    block.set_upload_location(value);
}

void    Config::parse_cgi_extension(Location & block, std::string & value)
{
    Log::log("inside parse cgi extension", DEBUG);
    if (block.get_cgi_extensions().empty() == false)
        throw std::runtime_error("double declaration of 'cgi_extension'");
        
    char str[value.size() + 1];
    std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    while (token)
    {
        // optional: implement checks if extension is accepted
        std::string extension = token;
            block.add_cgi_extension(extension);
        token = std::strtok(nullptr, " ");
    }
}

void    Config::parse_allowed_methods(Location & block, std::string & value)
{
    if (block.is_get() == true || block.is_post() == true || block.is_del() == true)
        throw std::runtime_error("double occurrence of 'allowed_methods'");

    char str[value.size() + 1];
    std::strcpy(str, value.c_str());
    char *token = std::strtok(str, " ");
    while (token)
    {
        if (std::strcmp(token, "GET") == 0 && block.is_get() == false)
            block.set_get(true);
        else if (std::strcmp(token, "POST") == 0 && block.is_post() == false)
            block.set_post(true);
        else if (std::strcmp(token, "DELETE") == 0 && block.is_del() == false)
            block.set_del(true);
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

bool Config::compare_prefix(const Location & a, const Location & b)
{
    return (a.get_path().size() > b.get_path().size());
}

bool Config::same_prefix(const Location & a, const Location & b)
{
    return (a.get_path() == b.get_path());
}