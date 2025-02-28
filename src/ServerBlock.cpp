
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

const std::map<int, std::string>&  ServerBlock::get_error_pages() const
{
    return (_error_pages);
};

unsigned int    ServerBlock::get_max_client_body() const
{
    return (_max_client_body);
};

const std::vector<Location>&   ServerBlock::get_locations() const
{   
    return (_locations);
}

void    ServerBlock::set_port(int port)
{
    _port = port;
};

void    ServerBlock::set_host(in_addr_t host)
{
    _host = host;
};

void    ServerBlock::add_error_page(int code, std::string path)
{
    _error_pages[code] = path;
};

void    ServerBlock::set_max_client_body(unsigned int size)
{
    _max_client_body = size;
};

void    ServerBlock::add_location(Location & location)
{
    _locations.push_back(location);
};

std::ostream &operator<<(std::ostream &os, const ServerBlock &server_block)
{
    os << "ServerBlock:" << std::endl;
    os << "  port: " << server_block.get_port() << std::endl;
    os << "  host: " << Config::ft_inet_ntoa(server_block.get_host()) << std::endl;
    os << "  max_client_body: " << server_block.get_max_client_body() << std::endl;
    os << "  error_pages: " << std::endl;
    for (std::map<int, std::string>::const_iterator it = server_block.get_error_pages().begin(); it != server_block.get_error_pages().end(); ++it)
            std::clog << "    " << it->first << " " << it->second << std::endl;
    os << "Locations:" << std::endl;
    for (std::vector<Location>::const_iterator it = server_block.get_locations().begin(); it != server_block.get_locations().end(); ++it)
        os << *it;
    return (os);
};

void ServerBlock::validate_locations()
{
    std::sort(_locations.begin(), _locations.end(), Config::compare_prefix); // sort by prefix: longest -> shortest
    std::vector<Location>::iterator location_it, tmp;
    for (location_it = _locations.begin(); location_it != _locations.end(); ++location_it)
    {
        if ((location_it + 1) != _locations.end() && Config::same_prefix(*location_it, *(location_it + 1)))  // error for same prefixes
            throw std::runtime_error("locations with same prefix cannot override each other");

        // optional: check if root / exists
        // optional: check if location is empty?
    }
};


