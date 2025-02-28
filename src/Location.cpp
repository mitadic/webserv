
#include "Location.hpp"

Location::Location() : _upload_allowed(false), _get(false), _post(false), _del(false), _autoindex(false), _redirect(0, "") {};

std::string Location::get_path() const
{
    return (_path);
};

std::string Location::get_root() const
{
    return (_root);
};

std::string Location::get_index() const
{
    return (_index);
};

std::string Location::get_upload_location() const
{
    return (_upload_location);
};

bool        Location::is_upload_allowed() const
{
    return (_upload_allowed);
};

bool        Location::is_get() const
{
    return (_get);
};

bool        Location::is_post() const
{
    return (_post);
};

bool        Location::is_del() const
{
    return (_del);
};

bool        Location::is_autoindex() const
{
    return (_autoindex);
};

const std::vector<std::string>& Location::get_cgi_extensions() const
{
    return (_cgi_extensions);
};

const std::pair<int, std::string>& Location::get_redirect() const
{
    return (_redirect);
};


void Location::set_path(std::string path)
{
    _path = path;
};

void Location::set_root(std::string root)
{
    _root = root;
};

void Location::set_index(std::string index)
{
    _index = index;
};

void Location::set_upload_location(std::string upload_location)
{
    _upload_location = upload_location;
};

void Location::set_upload_allowed(bool upload_allowed)
{
    _upload_allowed = upload_allowed;
};

void Location::set_get(bool get)
{
    _get = get;
};

void Location::set_post(bool post)
{
    _post = post;
};

void Location::set_del(bool del)
{
    _del = del;
};

void Location::set_autoindex(bool autoindex)
{
    _autoindex = autoindex;
};

void Location::add_cgi_extension(std::string extension)
{
    _cgi_extensions.push_back(extension);
};

void Location::set_redirect(int code, std::string url)
{
    _redirect = std::make_pair(code, url);
};

std::ostream &operator<<(std::ostream &os, const Location &location_block)
{
    os << "Location:" << std::endl;
    os << "  path: " << location_block.get_path() << std::endl;
    os << "  root: " << location_block.get_root() << std::endl;
    os << "  index: " << location_block.get_index() << std::endl;
    os << "  upload_location: " << location_block.get_upload_location() << std::endl;
    os << "  upload_allowed: " << location_block.is_upload_allowed() << std::endl;
    os << "  get: " << location_block.is_get() << std::endl;
    os << "  post: " << location_block.is_post() << std::endl;
    os << "  del: " << location_block.is_del() << std::endl;
    os << "  autoindex: " << location_block.is_autoindex() << std::endl;
    os << "  cgi_extensions: ";
    for (std::vector<std::string>::const_iterator it = location_block.get_cgi_extensions().begin(); it != location_block.get_cgi_extensions().end(); ++it)
        os << *it << " ";
    os << std::endl;
    os << "  redirect: " << location_block.get_redirect().first << " " << location_block.get_redirect().second << std::endl;
    return (os);
}