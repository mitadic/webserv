
#include "ServerBlock.hpp"
#include "Location.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "Log.hpp"

ServerBlock::ServerBlock() :
	_port(0),
	_host(Config::ft_inet("255.255.255.255")),
	_max_client_body(0)
{}

ServerBlock::~ServerBlock()
{}

ServerBlock::ServerBlock(const ServerBlock& oth) :
	_port(oth._port),
	_host(oth._host),
	_error_pages(oth._error_pages),
	_max_client_body(oth._max_client_body),
	_locations(oth._locations)
{}

ServerBlock& ServerBlock::operator=(const ServerBlock& oth)
{
	if (this != &oth)
	{
		_port = oth._port;
		_host = oth._host;
		_error_pages = oth._error_pages;
		_max_client_body = oth._max_client_body;
		_locations = oth._locations;
	}
	return *this;
}

const uint16_t& ServerBlock::get_port() const
{
    return (_port);
};

const in_addr_t&   ServerBlock::get_host() const
{
    return (_host);
};

const std::map<int, std::string>&  ServerBlock::get_error_pages() const
{
    return (_error_pages);
};

const unsigned int&    ServerBlock::get_max_client_body() const
{
    return (_max_client_body);
};

const std::vector<Location>&   ServerBlock::get_locations() const
{
    return (_locations);
}

/**
 * @brief Sets and checks the port number.
 * @details The valid range is 0-65535, but 0 is reserved for
 * unspecified source/destination. Ports 1-1023 are "reserved" or
 * "priviledged" ports and only a priviledged process may bind to
 * a socket with this port. They are reserved for widely used
 * services e.g. HTTPS. We do not allow port 0, because we do
 * not want the system to assign a random port.
 */
void    ServerBlock::set_port(std::string port)
{
	if (_port != 0)
		throw std::runtime_error("server cannot have multiple ports");
	if (port.size() > 5 || !Config::has_only_digits(const_cast<char *>(port.c_str())))
		throw std::runtime_error("invalid port number" + port);
	if (std::atoi(port.c_str()) > 65535 || std::atoi(port.c_str()) <= 0)
		throw std::runtime_error("invalid port number range");
	_port = static_cast<uint16_t>(std::atoi(port.c_str()));
};

/**
 * @brief Transforms, sets and checks the ip address.
 * @details The valid range is "0.0.0.0"-"255.255.255.255".
 * We do not allow host "0.0.0.0", because we do
 * not want the system to listen on all available local interfaces.
 */
void    ServerBlock::set_host(std::string host)
{
	if (_host != Config::ft_inet("255.255.255.255"))
		throw std::runtime_error("host already declared");
	_host = Config::ft_inet(host.c_str());
	// optional: add more checks for accepted ip addresses
	if (_host == Config::ft_inet("255.255.255.255") || _host == Config::ft_inet("0.0.0.0"))
		throw std::runtime_error("invalid IP address");
};

void    ServerBlock::add_error_page(std::string page)
{
	std::string code, path;
	std::stringstream ss(page);
	if (!getline(ss, code, ' ') || !getline(ss, path))
		throw std::runtime_error("in server block: error_page directive requires 2 arguments");
	if (code.size() != 3 || !Config::has_only_digits(const_cast<char *>(code.c_str())))
		throw std::runtime_error("invalid error code " + code);
	Location::check_valid_path(path, ROOT);
	// optional: check if the path really does exist with stat()
	if (_error_pages.find(std::atoi(code.c_str())) != _error_pages.end())
		throw std::runtime_error("error page already exists for " + code);
	_error_pages[std::atoi(code.c_str())] = path;
};

void    ServerBlock::set_max_client_body(std::string size)
{
	if (_max_client_body != 0 || !Config::has_only_digits(const_cast<char *>(size.c_str()))
		|| (std::strtod(size.c_str(), NULL) > UINT32_MAX) || (std::strtod(size.c_str(), NULL) <= 0))
		throw std::runtime_error("max body size already declared or invalid size");
	// optional: change type or size
	_max_client_body = std::atol(size.c_str());
};

void    ServerBlock::add_location(Location & location)
{
    _locations.push_back(location);
};

std::ostream &operator<<(std::ostream &os, const ServerBlock &server_block)
{
	os << "ServerBlock:" << std::endl;
	os << "  port: " << server_block.get_port() << std::endl;
	os << "  host: " << Utils::ft_inet_ntoa(server_block.get_host()) << std::endl;
	os << "  max_client_body: " << server_block.get_max_client_body() << std::endl;
	os << "  error_pages: " << std::endl;
	for (std::map<int, std::string>::const_iterator it = server_block.get_error_pages().begin(); it != server_block.get_error_pages().end(); ++it)
			std::clog << "    " << it->first << " " << it->second << std::endl;
	os << "Locations:" << std::endl;
	for (std::vector<Location>::const_iterator it = server_block.get_locations().begin(); it != server_block.get_locations().end(); ++it)
		os << *it;
	return (os);
};

/**
 * @brief Sorts location blocks by prefix length.
 * Does not allow same prefixes.
 */
void ServerBlock::validate_locations()
{
	std::sort(_locations.begin(), _locations.end(), Location::compare_prefix); // sort by prefix: longest -> shortest
	std::vector<Location>::iterator location_it, tmp;
	for (location_it = _locations.begin(); location_it != _locations.end(); ++location_it)
	{
		if ((location_it + 1) != _locations.end() && Location::same_prefix(*location_it, *(location_it + 1)))  // error for same prefixes
			throw std::runtime_error("locations with same prefix cannot override each other");
		// optional: check if location is empty?
	}
};


