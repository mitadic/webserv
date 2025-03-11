
#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <vector>
# include <map>
# include <iostream>
# include <sstream>
# include <stdexcept>
# include <cstring>
# include <cstdlib>  // std::atoi()

typedef enum e_path {
    LOCATION,
    ROOT,
} t_path;

/**
 * @brief Class representing a location block in the config file
 * @var _path the location prefix (e.g. /images/)
 * @var _root the root directory of the prefix (e.g. /var/www/html)
 * @var _index file to serve if no file is specified in the request (index.html)
 * @var _upload_location the directory where uploaded files are stored
 * @var _upload_allowed whether uploads are allowed
 * @var _get whether GET requests are allowed
 * @var _post whether POST requests are allowed
 * @var _del whether DELETE requests are allowed
 * @var _autoindex whether autoindex is enabled
 * @var _cgi_extensions a vector of CGI extensions (e.g. .py)
 * @var _redirect a pair of an HTTP status code and a URL to redirect to
 *
 */
class Location {
    public:
        Location();

        // getters
        const std::string&                 get_path() const;
        const std::string&                 get_root() const;
        const std::string&                 get_index() const;
        const std::string&                 get_upload_location() const;
        bool                        is_upload_allowed() const;
        bool                        is_get() const;
        bool                        is_post() const;
        bool                        is_del() const;
        bool                        is_autoindex() const;
        const std::vector<std::string>&    get_cgi_extensions() const;
        const std::pair<int, std::string>& get_redirect() const;
		const int& get_redirect_count() const;

        // setters
        void set_path(std::string path);
        void set_root(std::string root);
        void set_index(std::string index);
        void set_upload(std::string upload_location);
        void set_allowed_methods(std::string input);
        void set_autoindex(std::string autoindex);
        void set_cgi_extensions(std::string extension);
        void set_redirect(std::string redirection);
		void increment_redirect_count();

        // methods
        static void check_valid_path(std::string & path, t_path type);
        static bool compare_prefix(const Location & a, const Location & b);
        static bool same_prefix(const Location & a, const Location & b);

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
		int							_redirect_count;
};

std::ostream &operator<<(std::ostream &os, const Location &location_block);

#endif
