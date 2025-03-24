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

std::vector<std::string> Utils::split(const std::string &str, char delim)
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

std::vector<std::string> Utils::split(const std::string &str, const std::string &delim)
{
	std::vector<std::string> tokens;
	size_t startPos = 0;
	size_t delimPos;

	while ((delimPos = str.find(delim, startPos)) != std::string::npos)
	{
		tokens.push_back(str.substr(startPos, delimPos - startPos));
		startPos = delimPos + delim.length();
	}

	// Add the last token after the last delimiter
	if (startPos < str.length())
	{
		tokens.push_back(str.substr(startPos));
	}

	return tokens;
}

bool Utils::fileExists(const std::string &file)
{
	return (access(file.c_str(), F_OK) != -1);
}

// stat returns 0 on success, and -1 on failure (e.g., if the file does not exist)
bool Utils::isDirectory(const std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}

std::vector<std::string> Utils::listDirectory(std::string filePath)
{
	DIR *dir = opendir(filePath.c_str());
	if (!dir)
	{
		throw std::runtime_error("Error opening directory: " + filePath);
	}

	std::vector<std::string> files;
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL)
	{
		files.push_back(ent->d_name);
	}
	closedir(dir);

	return files;
}

std::string Utils::readFile(const std::string &file)
{
	std::ifstream read_file(file.c_str(), std::ios::binary | std::ios::ate);
	if (!read_file)
	{
		throw std::runtime_error("Error opening the file: " + file);
	}

	std::streamsize file_size = read_file.tellg();
	read_file.seekg(0, std::ios::beg);

	std::string content(file_size, '\0');
	if (!read_file.read(&content[0], file_size))
	{
		throw std::runtime_error("Error reading the file: " + file);
	}

	return content;
}

// Find the server block that corresponds to the request based on port and host
// TODO: If no server block is found, return an error ?
const ServerBlock *Utils::getServerBlock(const Request &req, const std::vector<ServerBlock> &server_blocks)
{
	const ServerBlock *matchingServer = NULL;

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
const Location *Utils::getLocation(const Request &req, const ServerBlock *server)
{
	const std::vector<Location> &locations = server->get_locations();

	for (size_t i = 0; i < server->get_locations().size(); ++i)
	{
		// Log::log(" Comparing URI: " + req.get_request_uri() + " with location: " + locations[i].get_path(), DEBUG);
		if (req.get_request_uri().find(locations[i].get_path()) == 0)
		{
			// Log::log("Found matching location: " + locations[i].get_path(), DEBUG);
			return &locations[i];
		}
	}

	Log::log("No matching location found", ERROR);
	Log::log(req.get_request_str(), DEBUG);
	return NULL;
}

std::string Utils::sanitizeFilename(const std::string &filename)
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
bool Utils::uriIsSafe(const std::string &uri)
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

// URL decode
std::string Utils::url_decoder(const std::string &value)
{
	std::ostringstream decoded_str;
	decoded_str.fill('0');
	decoded_str << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
	{
		std::string::value_type c = (*i);

		if (c == '%')
		{
			// Get the next two characters
			std::string::value_type c1 = *(++i);
			std::string::value_type c0 = *(++i);

			// Convert hex to integer
			int num = 0;
			std::istringstream ss;
			ss.str(std::string() + c1 + c0);
			ss >> std::hex >> num;

			decoded_str << static_cast<char>(num);
		}
		else if (c == '+')
		{
			decoded_str << ' ';
		}
		else
		{
			decoded_str << c;
		}
	}

	return decoded_str.str();
}

std::string Utils::generateTimestamp()
{
	std::time_t t = std::time(NULL);
	std::tm *now = std::localtime(&t);
	std::stringstream timestamp;
	timestamp << std::setfill('0') << std::setw(4) << (now->tm_year + 1900) << '-'
			  << std::setfill('0') << std::setw(2) << (now->tm_mon + 1) << '-'
			  << std::setfill('0') << std::setw(2) << now->tm_mday << '_'
			  << std::setfill('0') << std::setw(2) << now->tm_hour << ':'
			  << std::setfill('0') << std::setw(2) << now->tm_min << ':'
			  << std::setfill('0') << std::setw(2) << now->tm_sec;
	return timestamp.str();
}

std::string Utils::ft_inet_ntoa(in_addr_t ip)
{
    std::string host;

    for (int i = 3; i >= 0; --i)
    {
        std::stringstream ss;
        ss << ((ip >> (i * 8)) & 255);
        host += ss.str();
        if (i > 0)
            host += ".";
    }
    return (host);
}
