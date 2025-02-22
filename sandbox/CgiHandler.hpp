/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mitadic <mitadic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 18:09:08 by pbencze           #+#    #+#             */
/*   Updated: 2025/02/20 16:30:23 by mitadic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sys/wait.h> // For waitpid()
#include <cstdlib>

#define IS_GET true
#define IS_POST false
#define CGI_PATH "cgi.py"
#define PRG_NAME "python3"
#define REQUEST_BODY "mayonnaise"

#define UNINITIALIZED -1

class CgiHandler {
public:
	CgiHandler()
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
	~CgiHandler() {}

	int pipe_fds[2];
	// int client_fd;
	std::string path;
	char *argv[4];

	void handle_cgi(std::vector<struct pollfd>& pfds, std::map<int, struct pollfd>& cgi_pipes)
	{
		if (pipe(pipe_fds) < 0)
		{
			std::perror("pipe");
			return;
		}

		pid_t pid = fork(); //create child
		if (pid < 0)
		{
			std::perror("fork");
			return;
		}

		if (pid == 0)
		{
			close(pipe_fds[0]);  // close the end not used in child right away
			dup2(pipe_fds[1], STDOUT_FILENO);
			close(pipe_fds[1]);
			execve("/usr/bin/python3", argv, NULL);
			std::perror("execve");
			return;
		}
		else
		{
			pid_t w;
			int wstatus;

			if (close(pipe_fds[1]) < 0)  // close the end used in child before waiting on child
			{
				std::perror("close");
				return;
			}
			w = waitpid(pid, &wstatus, 0);
			if (w < 0)
			{
				std::perror("waitpid");
				return; // placeholder
			}
	
			struct pollfd fd;
			fd.fd = pipe_fds[0]; // read end (bc we read)
			fd.events = POLLIN;
			pfds.push_back(fd);
			cgi_pipes[pipe_fds[0]] = fd;
		}
	}
private:
};
