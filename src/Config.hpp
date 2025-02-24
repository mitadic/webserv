
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>
# include <map>
# include <cstring>
# include <sstream>

class Config {
    public:
        //std::string host;
        static std::vector<ServerBlock> parse_config(std::string filename);

    private:
        std::stringstream _input;
};

class ServerBlock {
    public:
        int         socket_fd; // or do we store it only in pfds?
        int         port;
        in_addr_t   host;

        std::map<int, std::string>  error_pages;
        unsigned int                max_client_body;
        std::vector<Location>       locations;

    private:
};

class Location{
    public:
        Location();

        std::string         location;
        std::string         root;
        std::string         index;
        std::string         upload_location;
        bool                upload_allowed;
        bool                get, post, del;
        bool                autoindex;
        std::vector<std::string>    cgi_extensions;       
        std::map<int, std::string>  redirect;

    private:
};

#endif