/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbencze <pbencze@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 18:09:08 by pbencze           #+#    #+#             */
/*   Updated: 2025/03/20 18:00:10 by pbencze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <cstdlib>
#include <sys/wait.h> // For waitpid()

#define WHICH_PHP "usr/bin/php"
#define WHICH_PY "usr/bin/py"
#define WHICH_SH "usr/bin/sh"

#define UNINITIALIZED -1

#include "Types.hpp"

class Request;
class Location;

class CgiHandler {
	public:
		CgiHandler(const CgiHandler & oth);
		CgiHandler(const Request& req, const Location& loc, int method);
		~CgiHandler();

		int pipe_fds[2];

		std::string deduce_extension(const Request& req, const Location& loc) const;
		void identify_pathinfo_and_querystring(const std::string& s);
		void handle_cgi(std::vector<struct pollfd>& pfds, std::map<int, pfd_info>& pfd_info_map, int reqs_idx);

	private:
		CgiHandler();
		int _client_fd; // to track the cgi
		std::string _extension; // e.g. ".py"
		std::string _interpreter; // contains "/usr/bin/python3", "/usr/bin/php" or "/usr/bin/sh" -> needed by argc
		std::string _pathname; // e.g. /cgi-bin/scrip.php
		std::string _pathinfo;
		std::string _querystring;
		char **_argv; // argv[0] is the interpreter, argv[1] is the pathname, argv is terminated by NULL
		char **_envp; // contains meta-variables e.g. PATH_INFO, QUERY_STRING
		// TODO current working directory should be set to the directory containing the script; not sure where it is stored ?

		void set_env_variables(const Request& req, const Location& loc, int method);

};

/**
 * @brief These are the envp variables for the cgi needed by execve
 * according to RFC 3875
 * @example PATH_INFO extracted from the HTTP request line
 * @note not sure yet how much of this we will need
 */
// const char *cgi_envp_keys[] = {
// 	"AUTH_TYPE", // probably unused
// 	"CONTENT_LENGTH", // default: NULL or unset; size of message body
// 	"CONTENT_TYPE", // type of message body
// 	"GATEWAY_INTERFACE", // default: "CGI/1.1"
// 	"PATH_INFO", // default: NULL; path in the request line after the script itself and before the query string
// 	"PATH_TRANSLATED", // default: NULL; only exists if there is PATH_INFO; this will be: root + path + path-info
// 	"QUERY_STRING", // default: "";
// 	"REMOTE_ADDR", // default: ipv4 address of the client
// 	"REMOTE_HOST", // default: NULL
// 	"REMOTE_IDENT",
// 	"REMOTE_USER",
// 	"REQUEST_METHOD", // default: "GET" or "POST"
// 	"SCRIPT_NAME", // URI path before the path info segment
// 	"SERVER_NAME", // host
// 	"SERVER_PORT", // port
// 	"SERVER_PROTOCOL", // HTTP/1.1
// 	"SERVER_SOFTWARE" // e.g. "webserv"
// };
