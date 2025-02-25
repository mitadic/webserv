
#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <cstring>
# include <vector>
# include <map>

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
        std::pair<int, std::string>  redirect;

    private:
};

#endif