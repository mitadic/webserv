#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
	// request = REQUEST_BODY;
	path = CGI_PATH;
	if (IS_GET)
	{
		argv[0] = const_cast<char *>(PRG_NAME);
		argv[1] = const_cast<char *>(path.c_str());
		argv[2] = 0;
	}
	else if (IS_POST)
	{
		argv[0] = const_cast<char *>(PRG_NAME);
		argv[1] = const_cast<char *>(path.c_str());
		argv[2] = const_cast<char *>(REQUEST_BODY);
		argv[3] = 0;
	}
}

CgiHandler::~CgiHandler() {}

/** pushes_back new:
 * - pipe_fd to the pfds vector
 * - pfd_info to the pfd_info_map (with type CGI_PIPE)
*/
void CgiHandler::handle_cgi(std::vector<struct pollfd>& pfds, std::map<int, pfd_info>& pfd_info_map, int reqs_idx)
{
	if (pipe(pipe_fds) < 0)
	{
		perror("pipe");
		return;
	}

	pid_t pid = fork(); //create child
	if (pid < 0)
	{
		perror("fork");
		return;
	}

	if (pid == 0)
	{
		close(pipe_fds[0]);  // close the end not used in child right away
		dup2(pipe_fds[1], STDOUT_FILENO);
		close(pipe_fds[1]);
		execve("/usr/bin/python3", argv, NULL);
		perror("execve");
		return;
	}
	else
	{
		pid_t w;
		int wstatus;

		if (close(pipe_fds[1]) < 0)  // close the end used in child before waiting on child
		{
			perror("close");
			return;
		}
		w = waitpid(pid, &wstatus, 0);
		if (w < 0)
		{
			perror("waitpid");
			return; // placeholder
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