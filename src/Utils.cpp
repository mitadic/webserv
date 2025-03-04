/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 19:41:33 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 20:29:12 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include "Request.hpp"  // here in .cpp safe combo with fwd declaration in .hpp, include unsafe in .hpp 

bool Utils::fileExists (const std::string& file)
{
	return (access(file.c_str(), F_OK) != -1);
}

// stat returns 0 on success, and -1 on failure (e.g., if the file does not exist)
bool Utils::isDirectory(const std::string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}

std::string Utils::readFile(const std::string& file)
{
	std::ifstream read_file(file.c_str());
	if (!read_file.is_open())
	{
		throw std::runtime_error("Error opening the file '" + file + "'");
	}
	std::stringstream buffer;
	buffer << read_file.rdbuf();
	read_file.close();
	return buffer.str();
}

// Find the server block that corresponds to the request based on port and host
// TODO: If no server block is found, return an error ?
const ServerBlock* Utils::getServerBlock(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = NULL;

	for (size_t i = 0; i < server_blocks.size(); ++i)
	{
		std::cout << "sb.port, req.port, sb.host, req.host: " << server_blocks[i].get_port() << ", " << req.get_port() << ", " << server_blocks[i].get_host() << ", " << req.get_host() << std::endl;
		if (server_blocks[i].get_port() == req.get_port() && server_blocks[i].get_host() == req.get_host())
		{
			matchingServer = &server_blocks[i];
			break;
		}
	}
	if (!matchingServer)
	{
		throw std::runtime_error("No server block found for the request");
	}
	return matchingServer;
}

// Find the corresponding location in the server block based on the uri
// iterate though locations and return the longest matching location
const Location* Utils::getLocation(const Request& req, const ServerBlock* server)
{
	const Location* matchingLocation = NULL;
	std::string longestMatch = "";
	const std::vector<Location>& locations = server->get_locations();

	for (size_t i = 0; i < server->get_locations().size(); ++i)
	{
		if (req.get_request_uri().find(locations[i].get_path()) == 0 && locations[i].get_path().size() > longestMatch.size())
		{
			longestMatch = locations[i].get_path();
			matchingLocation = &locations[i];
		}
	}

	if (!matchingLocation)
	{
		throw std::runtime_error("No matching location found for the request");
	}
	return matchingLocation;
}