
#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <cstring>
# include <vector>
# include <map>
# include <iostream>

class Location {
    public:
        Location();

        // getters
        std::string                 get_path() const;
        std::string                 get_root() const;
        std::string                 get_index() const;
        std::string                 get_upload_location() const;
        bool                        is_upload_allowed() const;
        bool                        is_get() const;
        bool                        is_post() const;
        bool                        is_del() const;
        bool                        is_autoindex() const;
        const std::vector<std::string>&    get_cgi_extensions() const;
        const std::pair<int, std::string>& get_redirect() const;

        // setters
        void set_path(std::string path);
        void set_root(std::string root);
        void set_index(std::string index);
        void set_upload_location(std::string upload_location);
        void set_upload_allowed(bool upload_allowed);
        void set_get(bool get);
        void set_post(bool post);
        void set_del(bool del);
        void set_autoindex(bool autoindex);
        void add_cgi_extension(std::string extension);
        void set_redirect(int code, std::string url);

    private:
        std::string                 _path;
        std::string                 _root;
        std::string                 _index;
        std::string                 _upload_location;
        bool                        _upload_allowed;
        bool                        _get, _post, _del;
        bool                        _autoindex;
        std::vector<std::string>    _cgi_extensions;       
        std::pair<int, std::string> _redirect;
};

std::ostream &operator<<(std::ostream &os, const Location &location_block);

#endif