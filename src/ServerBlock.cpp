
#include "ServerBlock.hpp"
#include "Config.hpp"

ServerBlock::ServerBlock() : _port(-1), _max_client_body(0)
{
    _host = Config::ft_inet("255.255.255.255");
};

int ServerBlock::get_port() const
{
    return (_port);
};

in_addr_t   ServerBlock::get_host() const
{
    return (_host);
};

std::map<int, std::string>  ServerBlock::get_error_pages() const
{
    return (_error_pages);
};

unsigned int                ServerBlock::get_max_client_body() const
{
    return (_max_client_body);
};

void                        ServerBlock::set_port(int port)
{
    _port = port;
};

void                        ServerBlock::set_host(in_addr_t host)
{
    _host = host;
};

void                        ServerBlock::add_error_page(int code, std::string path)
{
    _error_pages[code] = path;
};

void                        ServerBlock::set_max_client_body(unsigned int size)
{
    _max_client_body = size;
};
