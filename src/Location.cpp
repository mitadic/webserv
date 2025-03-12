
#include "ServerBlock.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "Log.hpp"

Location::Location() :
	_upload_allowed(false),
	_get(false),
	_post(false),
	_del(false),
	_autoindex(false),
	_redirect(0, "")
{};

const std::string& Location::get_path() const
{
    return (_path);
};

const std::string& Location::get_root() const
{
    return (_root);
};

const std::string& Location::get_index() const
{
    return (_index);
};

const std::string& Location::get_upload_location() const
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
	std::string name;
	char str[path.size() + 1];
	std::strcpy(str, path.c_str());
	char *token = std::strtok(str, " ");
	name = token;
	if (!token)
		throw std::runtime_error("Location: missing location name");
	token = std::strtok(NULL, " ");
	if (token)
		throw std::runtime_error("Location: too many arguments");
	check_valid_path(name, LOCATION);
	_path = path;
};

void Location::set_root(std::string root)
{
	check_valid_path(root, ROOT);
	if (_root.empty() == false)
		throw std::runtime_error("double occurrence of 'root'");
	_root = root;
};

void Location::set_index(std::string index)
{
	if (_index.empty() == false)
		throw std::runtime_error("double occurrence of 'index'");
	if (index.find_first_of(" ") != std::string::npos)
		throw std::runtime_error("expected one argument for 'index'");
	_index = index;
};

void Location::set_upload(std::string upload)
{
	check_valid_path(upload, ROOT);
	if (_upload_location.empty() == false)
		throw std::runtime_error("double occurrence of 'upload'");
	_upload_location = upload;
	_upload_allowed = true;
};

void Location::set_allowed_methods(std::string methods)
{
	if (_get == true || _post == true || _del == true)
	throw std::runtime_error("double occurrence of 'allowed_methods'");

	char str[methods.size() + 1];
	std::strcpy(str, methods.c_str());
	char *token = std::strtok(str, " ");
	while (token)
	{
		if (std::strcmp(token, "GET") == 0 && _get == false)
			_get = true;
		else if (std::strcmp(token, "POST") == 0 && _post == false)
			_post = true;
		else if (std::strcmp(token, "DELETE") == 0 && _del == false)
			_del = true;
		else
			throw std::runtime_error("Unknown or duplicate method in location block");
		token = std::strtok(NULL, " ");
	}
};

void Location::set_autoindex(std::string autoindex)
{
	if (_autoindex == true)
		throw std::runtime_error("double declaration of 'autoindex'");
	if (autoindex == "on")
		_autoindex = true;
	else if (autoindex != "off")
		throw std::runtime_error("in location block: autoindex requires value 'on' or 'off'");
};

void Location::set_cgi_extensions(std::string extensions)
{
	if (_cgi_extensions.empty() == false)
		throw std::runtime_error("double declaration of 'cgi_extension'");

	char str[extensions.size() + 1];
	std::strcpy(str, extensions.c_str());
	char *token = std::strtok(str, " ");
	while (token)
	{
		// optional: implement checks if extension is accepted
		std::string extension = token;
			_cgi_extensions.push_back(extension);
		token = std::strtok(NULL, " ");
	}
};

void Location::set_redirect(std::string redirection)
{
	if (_redirect.second.empty() == false)
		throw std::runtime_error("double declaration of 'return'");
	std::string code, url;
	std::stringstream ss(redirection);
	if (!getline(ss, code, ' ') || !getline(ss, url) || (url.find(' ') != std::string::npos)
		|| !Config::has_only_digits(const_cast<char *>(code.c_str())))
		throw std::runtime_error("in location block: return directive has wrong arguments");
	if (_path == url)
		throw std::runtime_error("directive redirects to itself"); // this could create an infinite loop
	int status = std::atoi(code.c_str());
	if (status != 301 && status != 302 && status != 307 && status != 308)
		throw std::runtime_error("return directive contains unknown status code");
	_redirect = std::make_pair(status, url);
};

/**
 * @brief Checks if root or location name have a valid syntax
 */
void Location::check_valid_path(std::string & path, t_path type)
{
	if (path[0] != '/')
		throw std::runtime_error("invalid path: absolute path has to start with '/'");
	if (type == ROOT && path[path.size() - 1] == '/')
		throw std::runtime_error("invalid path: root should not end with '/'");
	if (path.find("//") != std::string::npos || path.find_first_of("*?$\\% ") != std::string::npos)
		throw std::runtime_error("invalid path: contains '//' or one of the characters ' *?$\\%'");
};

bool Location::compare_prefix(const Location & a, const Location & b)
{
    return (a.get_path().size() > b.get_path().size());
}

bool Location::same_prefix(const Location & a, const Location & b)
{
    return (a.get_path() == b.get_path());
}

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
};
