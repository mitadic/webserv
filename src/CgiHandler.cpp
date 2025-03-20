#include "CgiHandler.hpp"
#include "Exceptions.hpp"
#include "Location.hpp"
#include "Request.hpp"

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

	// TODO add checks for valid request uri (maybe already in request parsing?)
	size_t pos = uri.find(deduce_extension(req, loc));
	_pathname = uri.substr(0, pos + _extension.length());
	identify_pathinfo_and_querystring(uri.substr(pos + _extension.length()));

	_argv = new char*[3];
	_argv[0] = const_cast<char *>(_interpreter.c_str()); // check later if cons_cast is ok
	_argv[1] = const_cast<char *>(_pathname.c_str());
	_argv[2] = NULL;

	set_env_variables(req, loc, method);
}

CgiHandler::CgiHandler(const CgiHandler & oth) {
	(void)oth;
	// TODO
	return ;
}

CgiHandler::~CgiHandler() {
	if (_argv)
		delete [] _argv;
	if (_envp)
	{
		int i = -1;
		while (_envp[++i])
			delete _envp[i];
		delete [] _envp;
	}
}

std::string CgiHandler::deduce_extension(const Request& req, const Location& loc) const
{
	std::string uri = req.get_request_uri();
	std::vector<std::string>::const_iterator it;
	for (it = loc.get_cgi_extensions().begin(); it != loc.get_cgi_extensions().end(); it++)
	{
		if (req.get_request_uri().find(*it))
			return (*it);
	}
	return "";
}

void CgiHandler::identify_pathinfo_and_querystring(const std::string& s)
{
	if (s.empty())
		return;

	size_t qm_pos = s.find("?");
	if (qm_pos != std::string::npos)
	{
		_querystring = s.substr(qm_pos);
		_pathinfo = s.substr(0, qm_pos);
	}
	else
		_pathinfo = s.substr(0);
	// check allowed syntax
	// check allowed syntax
	// throw error if larger than 2?
}

static char **vector_to_2d_array(std::vector<std::string> v)
{
	char **env = new char* [v.size()];
	for (size_t i = 0; i < v.size(); i++)
	{
		env[i] = new char [v[i].size() + 1];
		std::strcpy(env[i], v[i].c_str());
	}
	return (env);
}

void CgiHandler::set_env_variables(const Request& req, const Location& loc, int method)
{
	std::vector<std::string> env_vector;

	if (method == GET)
		env_vector.push_back("REQUEST_METHOD=GET");
	else if (method == POST)
	{
		env_vector.push_back("REQUEST_METHOD=POST");
		// env_vector.push_back("CONTENT_LENGTH=" + req.get_content_length()); // TODO transform into string
		env_vector.push_back("CONTENT_TYPE=" + static_cast<std::string>(req.get_content_type()));
	}
	if (!_pathinfo.empty())
	{
		env_vector.push_back("PATH_INFO=" + _pathinfo); // TODO extract path info
		env_vector.push_back("PATH_TRANSLATED=" + (loc.get_root() + _pathname + _pathinfo));
	}
	if (!_querystring.empty())
		env_vector.push_back("QUERY_STRING=" + _querystring);
	env_vector.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env_vector.push_back("SCRIPT_NAME=" + _pathname.substr(_pathname.find_last_of('/') + 1)); // TODO extract URI path before the path info segment
	// env_vector.push_back("SERVER_NAME=" + req.get_host());
	// env_vector.push_back("SERVER_PORT=" + req.get_port());  // TODO transform into string
	env_vector.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_vector.push_back("SERVER_SOFTWARE=Webserv");

	_envp = vector_to_2d_array(env_vector); // TODO
}

/** pushes_back new:
 * - pipe_fd to the pfds vector
 * - pfd_info to the pfd_info_map (with type CGI_PIPE)
*/
void CgiHandler::handle_cgi(std::vector<struct pollfd>& pfds, std::map<int, pfd_info>& pfd_info_map, int reqs_idx)
{
	if (pipe(pipe_fds) < 0)
	{
		std::ostringstream oss; oss << "pipe: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
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
		// actually it first needs to read the message body from stdin
		close(pipe_fds[0]);  // close the end not used in child right away
		dup2(pipe_fds[1], STDOUT_FILENO);
		close(pipe_fds[1]);
		execve(_interpreter.c_str(), _argv, _envp); // path is /usr/bin/python3
		std::ostringstream oss; oss << "execve: " << std::strerror(errno);
		Log::log(oss.str(), WARNING);
		throw CgiException();
	}
	else
	{
		pid_t w;
		int wstatus;

		if (close(pipe_fds[1]) < 0)  // close the end used in child before waiting on child
		{
			std::ostringstream oss; oss << "close: " << std::strerror(errno);
			Log::log(oss.str(), WARNING);
			throw CgiException();
		}
		w = waitpid(pid, &wstatus, 0);
		if (w < 0)
		{
			std::ostringstream oss; oss << "waitpid: " << std::strerror(errno);
			Log::log(oss.str(), WARNING);
			throw CgiException();
		}

		// will need more params and stuff to be able to (1) rm req, (2) set 504 response and POLLOUT

		struct pollfd fd;
		fd.fd = pipe_fds[0]; // read end (bc we read)
		fd.events = POLLIN;
		pfds.push_back(fd);

		pfd_info info = {};
		info.type = CGI_PIPE;
		info.reqs_idx = reqs_idx;
		pfd_info_map[pipe_fds[0]] = info;
	}
}
