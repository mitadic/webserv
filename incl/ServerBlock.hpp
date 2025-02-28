
#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <vector>
# include <map>
# include <arpa/inet.h>

# include "Location.hpp"

class ServerBlock {
    public:
        ServerBlock();

        // getters
        int                                 get_port() const;
        in_addr_t                           get_host() const;
        const std::map<int, std::string>&   get_error_pages() const;
        unsigned int                        get_max_client_body() const;
        const std::vector<Location>&        get_locations() const;

        // setters
        void                        set_port(int port);
        void                        set_host(in_addr_t host);
        void                        add_error_page(int code, std::string path);
        void                        set_max_client_body(unsigned int size);
        void                        add_location(Location & location);

        // methods
        void    validate_locations();

    private:
        //int         socket_fd; // or do we store it only in pfds?
        int                         _port;
        in_addr_t                   _host;
        std::map<int, std::string>  _error_pages;
        unsigned int                _max_client_body;
        std::vector<Location>       _locations;
};

std::ostream &operator<<(std::ostream &os, const ServerBlock &server_block);
    
#endif