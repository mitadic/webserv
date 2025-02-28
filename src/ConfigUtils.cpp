
#include "Config.hpp"

in_addr_t   Config::ft_inet(std::string host)
{
    char *host_c = const_cast<char *>(host.c_str());
    host_c = std::strtok(host_c, ".");
    uint32_t ip = 0;
    int i = 4;
    while (host_c)
    {
        --i;
        if (std::strlen(host_c) > 3 || std::strlen(host_c) < 1 || !has_only_digits(host_c) 
            || (std::strlen(host_c) > 1 && host_c[0] == '0') 
            || std::atoi(host_c) > 255 || std::atoi(host_c) < 0)
            throw std::runtime_error("invalid IP address");
        ip |= std::atoi(host_c) << (i * 8);
        host_c = std::strtok(nullptr, ".");
    }
    if (i < 0)
        throw std::runtime_error("invalid IP address");
    return (ip);
}

std::string Config::ft_inet_ntoa(in_addr_t ip)
{
    std::string host;
    for (int i = 3; i >= 0; --i)
    {
        std::stringstream ss;
        ss << std::to_string((ip >> (i * 8)) & 255);
        host += ss.str();
        if (i > 0)
            host += ".";
    }
    return (host);
}

/**
 * @brief Checks if root or location name have a valid syntax
 */
void Config::check_valid_path(std::string & path, t_path type)
{
    if (path[0] != '/')
        throw std::runtime_error("Invalid path: absolute path has to start with '/'");
    if (type == ROOT && path[path.size() - 1] == '/')
        throw std::runtime_error("Invalid path: root should not end with '/'");
    if (path.find("//") != std::string::npos || path.find_first_of("*?$\\% ") != std::string::npos)
        throw std::runtime_error("Invalid path: contains '//' or one of the characters ' *?$\\%'");
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
    result = result.substr(first, (last - first + 1));

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