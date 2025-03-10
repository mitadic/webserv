
#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <vector>
# include <map>
# include <algorithm>
# include <string>
# include <sstream>
# include <cstdlib>  // std::atoi
# include <stdint.h> // UINT32_MAX
# include <arpa/inet.h>

class Location;

/**
 * @brief Class respresenting a server block in the config file
 * @var _port the port the server listens on (0 - 65535)
 * @var _host the host the server listens on (0.0.0.0 - 255.255.255.254)
 * @var _error_pages a map of error codes and their corresponding paths
 * @var _max_client_body the maximum size of the client body (> 0)
 * @var _locations a vector of location blocks
 */
class ServerBlock {
	public:
		ServerBlock();
		~ServerBlock();
		ServerBlock(const ServerBlock& oth);
		ServerBlock& operator=(const ServerBlock& oth);

		// getters
		const uint16_t&							get_port() const;
		const in_addr_t&							get_host() const;
		const std::map<int, std::string>&	get_error_pages() const;
		const unsigned int&						get_max_client_body() const;
		const std::vector<Location>&		get_locations() const;

		// setters
		void						set_port(std::string port);
		void						set_host(std::string host);
		void						add_error_page(std::string page);
		void						set_max_client_body(std::string size);
		void						add_location(Location & location);

		// methods
		void	validate_locations();

	private:
		//int         socket_fd; // or do we store it only in pfds?
		uint16_t					_port;
		in_addr_t					_host;
		std::map<int, std::string>	_error_pages;
		unsigned int				_max_client_body;
		std::vector<Location>		_locations;
};

std::ostream &operator<<(std::ostream &os, const ServerBlock &server_block);

#endif
