
#include "Config.hpp"

in_addr_t   Config::ft_inet(const std::string& host)
{
     char *host_c = const_cast<char *>(host.c_str());
    host_c = std::strtok(host_c, ".");
    in_addr_t ip = 0;
    int i = 4;
    while (host_c)
    {
        --i;
        if (std::strlen(host_c) > 3 || std::strlen(host_c) < 1 || !has_only_digits(host_c)
            || (std::strlen(host_c) > 1 && host_c[0] == '0')
            || std::atoi(host_c) > 255 || std::atoi(host_c) < 0)
            throw std::runtime_error("invalid IP address");
        ip |= std::atoi(host_c) << (i * 8);
        host_c = std::strtok(NULL, ".");
    }
    if (i < 0)  // will never be reached
        throw std::runtime_error("invalid IP address");
    return (ip); //does not convert from host byte to network byte order, so we gain readability and uniform handling in listeners setup
}

std::string Config::ft_inet_ntoa(in_addr_t ip)
{
    std::string host;

    for (int i = 3; i >= 0; --i)
    {
        std::stringstream ss;
        ss << ((ip >> (i * 8)) & 255);
        host += ss.str();
        if (i > 0)
            host += ".";
    }
    return (host);
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
