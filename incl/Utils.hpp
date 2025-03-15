/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:37 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/15 14:57:10 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
#include <dirent.h>
#include <iomanip>
#include "ServerBlock.hpp"

class Request;

namespace Utils
{
	std::vector<std::string> split(const std::string& str, char delim);
	std::vector<std::string> split(const std::string& str, const std::string& delim);

	bool fileExists (const std::string& file);
	bool isDirectory(const std::string& path);

	std::vector<std::string> listDirectory(std::string filePath);

	std::string readFile(const std::string& file);

	const ServerBlock* getServerBlock(const Request& req, const std::vector<ServerBlock>& server_blocks);
	const Location* getLocation(const Request& req, const ServerBlock* server);

	std::string sanitizeFilename(const std::string& filename);
	bool uriIsSafe(const std::string& uri);

	std::string host_to_str(const in_addr_t);

	std::string url_decoder(const std::string &value);
}