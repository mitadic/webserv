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

#include "Types.hpp"

class CgiHandler {
public:
	CgiHandler();
	~CgiHandler();

	int pipe_fds[2];
	// int client_fd;
	std::string path;
	char *argv[4];

	void handle_cgi(std::vector<struct pollfd>& pfds, std::map<int, pfd_info>& pfd_info_map, int reqs_idx);

private:

};
