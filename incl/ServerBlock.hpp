
#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <vector>
# include <map>
# include <arpa/inet.h>

# include "Location.hpp"

class ServerBlock {
    public:
        ServerBlock();
        //int         socket_fd; // or do we store it only in pfds?
        int         port;
        in_addr_t   host;

        std::map<int, std::string>  error_pages;
        unsigned int                max_client_body;
        std::vector<Location>       locations;

    private:
};

#endif