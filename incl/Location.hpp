
#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <cstring>
# include <vector>
# include <map>

class Location{
    public:
        Location();

    private:
        std::string                 _location;
        std::string                 _root;
        std::string                 _index;
        std::string                 _upload_location;
        bool                        _upload_allowed;
        bool                        _get, post, del;
        bool                        _autoindex;
        std::vector<std::string>    _cgi_extensions;       
        std::pair<int, std::string> _redirect;
    };

#endif