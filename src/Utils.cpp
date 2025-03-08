/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mitadic <mitadic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 19:41:33 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/08 16:50:03 by aarponen         ###   ########.fr       */

/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include "Request.hpp"
#include "Location.hpp"

std::vector<std::string> Utils::split(const std::string& str, char delim)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delim))
	{
		tokens.push_back(token);
	}
	return tokens;
}

std::vector<std::string> Utils::split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t startPos = 0;
	size_t delimPos;

	while ((delimPos = str.find(delim, startPos)) != std::string::npos) {
		tokens.push_back(str.substr(startPos, delimPos - startPos));
		startPos = delimPos + delim.length();
	}

	// Add the last token after the last delimiter
	if (startPos < str.length()) {
		tokens.push_back(str.substr(startPos));
	}

	return tokens;
}

bool Utils::fileExists (const std::string& file)
{
	return (access((file).c_str(), F_OK) != -1);
}

// stat returns 0 on success, and -1 on failure (e.g., if the file does not exist)
bool Utils::isDirectory(const std::string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}

std::vector<std::string> Utils::listDirectory(std::string filePath)
{
	DIR* dir = opendir(filePath.c_str());
	if (!dir)
	{
		throw std::runtime_error("Error opening directory: " + filePath);
	}

	std::vector<std::string> files;
	struct dirent* ent;
	while ((ent = readdir(dir)) != NULL)
	{
		files.push_back(ent->d_name);
	}
	closedir(dir);

	return files;
}

std::string Utils::readFile(const std::string& file)
{
	std::ifstream read_file(file.c_str());
	if (!read_file.is_open())
	{
		throw std::runtime_error("Error opening the file: " + file);
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
// Locations are sorted by length, so the first match is the longest match
const Location* Utils::getLocation(const Request& req, const ServerBlock* server)
{
	const std::vector<Location>& locations = server->get_locations();

	for (size_t i = 0; i < server->get_locations().size(); ++i)
	{
		if (req.get_request_uri().find(locations[i].get_path()) == 0)
			return &locations[i];
	}

	Log::log("No matching location found for the request", WARNING);
	std::cout << "Request URI: " << req.get_request_uri() << std::endl;
	throw std::runtime_error("No matching location found for the request");
}



std::string Utils::sanitizeFilename(const std::string& filename)
{
	std::string sanitized;
	for (size_t i = 0; i < filename.size(); ++i)
	{
		char c = filename[i];
		if (isalnum(c) || strchr("._-", c) != NULL)
			sanitized += c;
		else
			sanitized += '_';
	}
	return sanitized;
}

// ensure the uri does not travel up above the root directory
bool Utils::uriIsSafe(const std::string& uri)
{
	std::istringstream stream(uri);
	std::string segment;
	int depth = 0;

	while (std::getline(stream, segment, '/'))
	{
		if (segment == "..")
		{
			if (depth > 0)
				--depth;
			else
				return false; // Invalid, because trying to go above root directory
		}
		else if (!segment.empty() && segment != ".")
			++depth;
	}

	return true;
}

std::string Utils::host_to_str(const in_addr_t num)
{
	std::stringstream ss;
	for (int i = 0; i < 4; i++)
	{
		ss << (num >> (24 - (8 * i)) & 255);
		if (i != 3)
			ss << ".";
	}
	return ss.str();
}
