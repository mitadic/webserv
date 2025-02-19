/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbencze <pbencze@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 18:09:08 by pbencze           #+#    #+#             */
/*   Updated: 2025/02/19 19:52:17 by pbencze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

#define IS_CGI true
#define GET true
#define POST false
#define CGI_PATH "sandbox/cgi.py"
#define REQUEST_BODY "hello"

class Cgi {
	public:
	Cgi()
	{
		cgi_flag = IS_CGI;
		pipe = new int[2];
		client_fd = 0;
		request = REQUEST_BODY;
		path = CGI_PATH;
		if (GET)
		{
			argv[0] = const_cast<char *>(path.c_str());
			argv[1] = 0;
		}
		else if (POST)
		{
			argv[0] = const_cast<char *>(path.c_str());
			argv[1] = const_cast<char *>(request.c_str());
			argv[2] = 0;
		}
	}
	~Cgi()
	{
		if (pipe)
			delete pipe;
	}

	bool cgi_flag;
	std::string response;
	int *pipe;
	int client_fd;
	std::string request;
	std::string path;
	char *argv[3];
};
