/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 12:38:37 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 20:29:23 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
#include "ServerBlock.hpp"
// #include "Request.hpp"

class Request;

namespace Utils
{
	bool fileExists (const std::string& file);
	bool isDirectory(const std::string& path);
	std::string readFile(const std::string& file);
	const ServerBlock* getServerBlock(const Request& req, const std::vector<ServerBlock>& server_blocks);
	const Location* getLocation(const Request& req, const ServerBlock* server);
}