/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 17:14:55 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestProcessor.hpp"

// ------- Helper functions ------------

std::string sendContent(const std::string& file, const std::string& mime_type)
{
	std::string body = Utils::readFile(file);

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Type: " << mime_type << "\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "\r\n"
				<< body;

	return response.str();
}

std::string sendErrorPage(const std::string& file)
{
	std::string body = Utils::readFile(file);

	std::ostringstream response;
	response << "HTTP/1.1 404 Not Found\r\n"
				<< "Content-Type: text/html\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "\r\n"
				<< body;
	return response.str();
}

// Find the server block that corresponds to the request based on port and host
// TODO: If no server block is found, return an error ?
const ServerBlock* getServerBlock(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = NULL;

	for (size_t i = 0; i < server_blocks.size(); ++i)
	{
		if (server_blocks[i].port == req.port && server_blocks[i].host == req.host)
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
const Location* getLocation(const Request& req, const ServerBlock* server)
{
	const Location* matchingLocation = NULL;
	std::string longestMatch = "";

	for (size_t i = 0; i < server->locations.size(); ++i)
	{
		if (req.uri.find(server->locations[i].location) == 0 && server->locations[i].location.size() > longestMatch.size())
		{
			longestMatch = server->locations[i].location;
			matchingLocation = &server->locations[i];
		}
	}

	if (!matchingLocation)
	{
		throw std::runtime_error("No matching location found for the request");
	}
	return matchingLocation;
}


// ------- METHODS --------------

std::string RequestProcessor::handleMethod(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	switch (req.method)
	{
	case GET:
		return processGet(req, server_blocks);
	case POST:
		return processPost(req, server_blocks);
	case DELETE:
		return processDelete(req, server_blocks);
	default:
		throw std::runtime_error("Unsupported HTTP method");
	}
}

// GET method
// get the server block that corresponds to the request
// get the location that corresponds to the request
// check if the method is allowed in the location
// check if the file exists
// if it's a directory, show default index file
// - if index file doesn't exist, show directory content if autoindex is on
// - if autoindex is off, return 403 error page
// if it's a file, return the file content
// if it doesn't exist, return 404 error page
std::string RequestProcessor::processGet(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = getServerBlock(req, server_blocks);
	const Location* matchingLocation = getLocation(req, matchingServer);

	if (!matchingLocation->get)
	{
		std::string errorPage;
		auto it = matchingServer->error_pages.find(405);
		if (it != matchingServer->error_pages.end())
			errorPage = it->second;
		else
			errorPage = "www/405.html"; // TODO:Default 405 page
		return sendErrorPage(errorPage);
	}

	std::string filePath = matchingLocation->root + req.uri;

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			if (Utils::fileExists(filePath + matchingLocation->index))
			{
				return sendContent(filePath + matchingLocation->index, req.mime_type); // Return the index file
			}
			else
			{
				if (matchingLocation->autoindex)
				{
					//TODO return directory listing -- is this something we want / need to implement?
				}
				else
				{
					std::string errorPage;
					auto it = matchingServer->error_pages.find(403);
					if (it != matchingServer->error_pages.end())
						errorPage = it->second;
					else
						errorPage = "www/403.html"; // TODO:Default 403 page
					return sendErrorPage(errorPage);
				}
			}
		}
		return sendContent(filePath, req.mime_type); // Return the file content
	}
	else
	{
		std::string errorPage;
		auto it = matchingServer->error_pages.find(404);
		if (it != matchingServer->error_pages.end())
			errorPage = it->second;
		else
			errorPage = "www/404.html"; // TODO:Default 404 page
		return sendErrorPage(errorPage);
	}
}

// write the post data to a file and return the file path to the user (?)
// error page if the file can't be created
std::string RequestProcessor::processPost(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	//TDO
}

// delete the file if it exists and deletions are allowed
// error page if the file can't be deleted
std::string RequestProcessor::processDelete(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = getServerBlock(req, server_blocks);
	const Location* matchingLocation = getLocation(req, matchingServer);

	std::string filePath = matchingLocation->root + req.uri;
}




