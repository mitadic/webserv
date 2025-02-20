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

#define IS_CGI true
#define GET true
#define POST false
#define CGI_PATH "cgi.py"
#define PRG_NAME "python3"
#define REQUEST_BODY "hello"

#define UNINITIALIZED -1

class CgiHandler {
public:
	CgiHandler() : client_fd(UNINITIALIZED)
	{
		cgi_flag = IS_CGI;
		// pipe = new int[2];
		// client_fd = 0;
		request = REQUEST_BODY;
		path = CGI_PATH;
		if (GET)
		{
			argv[0] = const_cast<char *>(PRG_NAME);
			argv[1] = const_cast<char *>(path.c_str());
			argv[2] = 0;
		}
		else if (POST)
		{
			argv[0] = const_cast<char *>(PRG_NAME);
			argv[1] = const_cast<char *>(path.c_str());
			argv[2] = const_cast<char *>(request.c_str());
			argv[3] = 0;
		}
	}
	~CgiHandler()
	{
		// if (pipe)
		// 	delete pipe;
	}

	bool cgi_flag;
	int pipe[2];
	int client_fd;
	std::string path;
	char *argv[4];
private:
};
