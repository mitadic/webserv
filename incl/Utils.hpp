/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mitadic <mitadic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:37 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/07 22:28:16 by mitadic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
#include "ServerBlock.hpp"

class Request;

namespace Utils
{
	std::vector<std::string> split(const std::string& str, char delim);
	std::vector<std::string> split(const std::string& str, const std::string& delim);

	bool fileExists (const std::string& file);
	bool isDirectory(const std::string& path);

	std::string readFile(const std::string& file);

	const ServerBlock* getServerBlock(const Request& req, const std::vector<ServerBlock>& server_blocks);
	const Location* getLocation(const Request& req, const ServerBlock* server);

	std::string sanitizeFilename(const std::string& filename);
	bool uriIsSafe(const std::string& uri);

	std::string host_to_str(const in_addr_t);
}