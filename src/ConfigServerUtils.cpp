
#include "Config.hpp"

void Config::parse_host(ServerBlock & block, std::string & value)
{
    if (block.get_host() != ft_inet("255.255.255.255"))
        throw std::runtime_error("host already declared");
    block.set_host(ft_inet(value.c_str()));
    // optional: add more checks for valid ip addresses
    if (block.get_host() == ft_inet("255.255.255.255"))
        throw std::runtime_error("invalid IP address");
}
void Config::parse_port(ServerBlock & block, std::string & value)
{
    if (block.get_port() != -1)
        throw std::runtime_error("server cannot have multiple ports");
    if (value.size() > 5 || !has_only_digits(const_cast<char *>(value.c_str())))
        throw std::runtime_error("invalid port number" + value);
    block.set_port(std::atoi(value.c_str()));
    if (block.get_port() > 65535 || block.get_port() < 0)
        throw std::runtime_error("invalid port number range");
    // optional: add more checks for valid ports
}

void Config::parse_error_page(ServerBlock & block, std::string & value)
{
    std::string code, path;
    std::stringstream ss(value);
    if (!getline(ss, code, ' ') || !getline(ss, path))
        throw std::runtime_error("in server block: error_page directive requires 2 arguments");
    if (code.size() != 3 || !has_only_digits(const_cast<char *>(code.c_str())))
        throw std::runtime_error("invalid error code " + code);
    check_valid_path(path, ROOT);
    // optional: check if the path really does exist with stat()
    if (block.get_error_pages().find(std::atoi(code.c_str())) != block.get_error_pages().end())
        throw std::runtime_error("error page already exists for " + code);
    block.add_error_page(std::atoi(code.c_str()), path);
}

void    Config::parse_client_body(ServerBlock & block, std::string & value)
{
    if (block.get_max_client_body() != 0 || !has_only_digits(const_cast<char *>(value.c_str()))
        || (std::strtod(value.c_str(), NULL) > UINT32_MAX))
        throw std::runtime_error("max body size already declared or invalid size");
    block.set_max_client_body(std::atol(value.c_str()));
    // optional: change type or size
}