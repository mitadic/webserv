#include "CgiHandler.hpp"
#include "Exceptions.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Utils.hpp"

/* CgiHandler helper function */
static char **vector_to_2d_array(const std::vector<std::string>& v)
{
	char **env = new char* [v.size() + 1];
	env[v.size()] = NULL;
	for (size_t i = 0; i < v.size(); i++)
	{
		env[i] = new char [v[i].size() + 1];
		std::strcpy(env[i], v[i].c_str());
	}
	return (env);
}

/* Parametrised constructor */
CgiHandler::CgiHandler(const Request& req, const Location& loc, int method) {

	std::string uri = req.get_request_uri();

	_client_fd = req.get_client_fd();
	_extension = deduce_extension(req, loc);
	if (_extension == ".py")
		_interpreter = WHICH_PY;
	else if (_extension == ".php")
		_interpreter =  WHICH_PHP;
	if (_extension == ".sh")
		_interpreter = WHICH_SH;

	size_t pos = uri.find(deduce_extension(req, loc));
	_pathname = "." + loc.get_root() + uri.substr(0, pos + _extension.length());
	_pathinfo = uri.substr(pos + _extension.length());
	_querystring = req.get_request_query_string();
	Log::log("uri: " + uri, SETUP);
	Log::log("pathinfo: " + _pathinfo, SETUP);
	Log::log("pathname: " + _pathname, SETUP);
	Log::log("query string: " + _querystring, SETUP);

	pipe_in[0] = UNINITIALIZED;
	pipe_in[1] = UNINITIALIZED;
	pipe_out[0] = UNINITIALIZED;
	pipe_out[1] = UNINITIALIZED;

	_argv = new char*[3];
	_argv[0] = const_cast<char *>(_interpreter.c_str());
	_argv[1] = const_cast<char *>(_pathname.c_str());
	_argv[2] = NULL;

	set_env_variables(req, loc, method);
}

CgiHandler::CgiHandler(const CgiHandler & oth) :
		_client_fd(oth._client_fd),
		_extension(oth._extension),
		_interpreter(oth._interpreter),
		_pathname(oth._pathname),
		_pathinfo(oth._pathinfo),
		_querystring(oth._querystring),
		_argv(NULL),
		_env_vector(oth._env_vector),
		_envp(NULL) {
	if (oth._argv)
	{
		_argv = new char*[3];
		_argv[0] = const_cast<char *>(_interpreter.c_str());
		_argv[1] = const_cast<char *>(_pathname.c_str());
		_argv[2] = NULL;
	}
	if (oth._envp)
		_envp = vector_to_2d_array(_env_vector);
	pipe_in[0] = oth.pipe_in[0];
	pipe_in[1] = oth.pipe_in[1];
	pipe_out[0] = oth.pipe_out[0];
	pipe_out[1] = oth.pipe_out[1];
	return ;
}

CgiHandler::~CgiHandler() {
	if (_argv)
	{
		delete [] _argv;
		_argv = NULL;
	}
	if (_envp)
	{
		int i = -1;
		while (_envp[++i])
			delete [] _envp[i];
		delete [] _envp;
		_envp = NULL;
	}
	if (pipe_in[0] != UNINITIALIZED)
		close(pipe_in[0]);
	if (pipe_in[1] != UNINITIALIZED)
		close(pipe_in[1]);
	if (pipe_out[0] != UNINITIALIZED)
		close(pipe_out[0]);
	if (pipe_out[1] != UNINITIALIZED)
		close(pipe_out[1]);
}

std::string CgiHandler::deduce_extension(const Request& req, const Location& loc) const
{
	std::string uri = req.get_request_uri();
	std::vector<std::string>::const_iterator it;
	for (it = loc.get_cgi_extensions().begin(); it != loc.get_cgi_extensions().end(); it++)
	{
		if (req.get_request_uri().find(*it) != std::string::npos)
			return (*it);
	}
	return "";
}

void CgiHandler::set_env_variables(const Request& req, const Location& loc, int method)
{
	(void)loc; // TODO maybe remove

	if (method == GET)
		_env_vector.push_back("REQUEST_METHOD=GET");
	else if (method == POST)
	{
		_env_vector.push_back("REQUEST_METHOD=POST");
		std::ostringstream oss; oss << "CONTENT_LENGTH=" << req.get_content_length(); _env_vector.push_back(oss.str());
		if (strcmp(req.get_content_type(), "multipart/form-data") == 0)
			_env_vector.push_back("CONTENT_TYPE=" + static_cast<std::string>(req.get_content_type()) + "; boundary=" + Utils::findBoundary(req));
		else
			_env_vector.push_back("CONTENT_TYPE=" + static_cast<std::string>(req.get_content_type()));
	}
	if (!_pathinfo.empty())
	{
		_env_vector.push_back("PATH_INFO=" + _pathinfo);
	}
	_env_vector.push_back("PATH_TRANSLATED=" + (_pathname.substr(0, _pathname.find_last_of('/')) + _pathinfo));
	if (!_querystring.empty())
		_env_vector.push_back("QUERY_STRING=" + _querystring);
	if (_extension == ".php")
	{
		// php-cgi interpreter relies on these additional env vars, seemingly crucially
		_env_vector.push_back("REDIRECT_STATUS=1");
		std::ostringstream oss; oss << "SCRIPT_FILENAME=" << _pathname;
		_env_vector.push_back(oss.str());
	}
	_env_vector.push_back("GATEWAY_INTERFACE=CGI/1.1");
	_env_vector.push_back("SCRIPT_NAME=" + _pathname.substr(_pathname.find_last_of('/') + 1));
	std::ostringstream oss; oss << "SERVER_PORT=" << req.get_port(); _env_vector.push_back(oss.str());
	_env_vector.push_back("SERVER_NAME=" + Utils::ft_inet_ntoa(req.get_host()));
	_env_vector.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_env_vector.push_back("SERVER_SOFTWARE=Webserv");

	_envp = vector_to_2d_array(_env_vector);
}

/** pushes_back new:
 * - pipe_fd to the pfds vector
 * - pfd_info to the pfd_info_map (with type CGI_PIPE)
*/
void CgiHandler::setup_cgi_get(std::vector<struct pollfd>& pfds, std::map<int, pfd_info>& pfd_info_map, Request *request)
{
	if (!Utils::fileExists(_pathname))
		throw RequestException(CODE_404);

	Log::log("Setting up exec of CGI get", DEBUG);
	if (pipe(pipe_out) < 0)
	{
		std::ostringstream oss; oss << "pipe: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw CgiException();
	}

	pid_t pid = fork(); //create child
	if (pid < 0)
	{
		std::ostringstream oss; oss << "fork: " << std::strerror(errno);
		Log::log(oss.str(), ERROR);
		throw CgiException();
	}

	if (pid == 0)
	{
		close(STDERR_FILENO); // to prevent seeing BROKEN_PIPE in the parent
		close(pipe_out[0]);  // close the end not used in child right away
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[1]);
		execve(_interpreter.c_str(), _argv, _envp); // path is /usr/bin/python3
		std::ostringstream oss; oss << "execve: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw CgiException();
	}
	else
	{
		if (close(pipe_out[1]) < 0)  // close the end used in child before waiting on child
		{
			std::ostringstream oss; oss << "close: " << std::strerror(errno);
			Log::log(oss.str(), WARNING);
			throw CgiException();
		}
		pipe_out[1] = UNINITIALIZED;

		struct pollfd fd;
		fd.fd = pipe_out[0]; // read end (bc we read)
		fd.events = POLLIN;
		pfds.push_back(fd);

		pfd_info info = {};
		info.type = CGI_PIPE_OUT;
		info.request = request;
		info.cgi_pid = pid;
		pfd_info_map[pipe_out[0]] = info;
	}
}


void CgiHandler::setup_cgi_post(std::vector<struct pollfd>& pfds, std::map<int, pfd_info>& pfd_info_map, Request *request)
{
	if (!Utils::fileExists(_pathname))
		throw RequestException(CODE_404);

	Log::log("Setting up exec of CGI post", DEBUG);
	if (pipe(pipe_in) < 0)
	{
		std::ostringstream oss; oss << "pipe(): " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw CgiException();
	}
	if (pipe(pipe_out) < 0)
	{
		std::ostringstream oss; oss << "pipe(): " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		close(pipe_in[0]);
		close(pipe_in[1]);
		pipe_in[0] = UNINITIALIZED;
		pipe_in[1] = UNINITIALIZED;
		throw CgiException();
	}

	pid_t pid = fork(); //create child
	if (pid < 0)
	{
		std::ostringstream oss; oss << "fork: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw CgiException();
	}

	if (pid == 0)
	{
		close(STDERR_FILENO); // to prevent seeing BROKEN_PIPE in the parent
		close(pipe_in[1]);
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[0]);

		close(pipe_out[0]);  // close the end not used in child right away
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[1]);

		execve(_interpreter.c_str(), _argv, _envp); // path is /usr/bin/python3
		std::ostringstream oss; oss << "execve: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw CgiException();
	}
	else
	{
		if (close(pipe_out[1]) < 0)  // close the end used in child before waiting on child
		{
			std::ostringstream oss; oss << "close: " << std::strerror(errno);
			Log::log(oss.str(), WARNING);
			throw CgiException();
		}
		pipe_out[1] = UNINITIALIZED;
		if (close(pipe_in[0]) < 0)  // update: close also this end used in child before waiting on child
		{
			std::ostringstream oss; oss << "close: " << std::strerror(errno);
			Log::log(oss.str(), WARNING);
			throw CgiException();
		}
		pipe_in[0] = UNINITIALIZED;

		struct pollfd fd_in;
		fd_in.fd = pipe_in[1];  // write end (bc we write)
		fd_in.events = POLLOUT;
		pfds.push_back(fd_in);

		struct pollfd fd_out;
		fd_out.fd = pipe_out[0]; // read end (bc we read)
		fd_out.events = POLLIN;
		pfds.push_back(fd_out);

		pfd_info info_in = {};
		info_in.type = CGI_PIPE_IN;
		info_in.request = request;
		info_in.cgi_pid = pid;
		pfd_info_map[pipe_in[1]] = info_in;

		pfd_info info_out = {};
		info_out.type = CGI_PIPE_OUT;
		info_out.request = request;
		info_out.cgi_pid = pid;
		pfd_info_map[pipe_out[0]] = info_out;
	}
}
