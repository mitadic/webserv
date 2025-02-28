/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 19:06:19 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestProcessor.hpp"

// ------- Helper functions ------------

std::string defineMime(const std::string& file)
{
	size_t pos = file.find_last_of('.');
	if (pos == std::string::npos || pos == file.length() - 1)
		return "application/octet-stream";

	std::string extension = file.substr(file.find_last_of('.') + 1);
	if (extension == "html")
		return "text/html";
	else if (extension == "css")
		return "text/css";
	else if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	else if (extension == "png")
		return "image/png";
	else if (extension == "gif")
		return "image/gif";
	else if (extension == "json")
		return "application/json";
	else if (extension == "pdf")
		return "application/pdf";
	else if (extension == "xml")
		return "application/xml";
	else if (extension == "xhtml")
		return "application/xhtml+xml";
	else if (extension == "txt")
		return "text/plain";
	else
		return "application/octet-stream";
}

std::string sendContent(const std::string& file)
{
	std::string body = Utils::readFile(file);

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Type: " << defineMime(file) << "\r\n"
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
const Location* getLocation(const Request& req, const ServerBlock* server)
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


// ------- METHODS --------------

std::string RequestProcessor::handleMethod(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	switch (req.get_method())
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

	if (!matchingLocation->is_get())
	{
		std::string errorPage;
		auto it = matchingServer->get_error_pages().find(405);
		if (it != matchingServer->get_error_pages().end())
			errorPage = it->second;
		else
			errorPage = "www/405.html"; // TODO:Default 405 page
		return sendErrorPage(errorPage);
	}

	std::string filePath = matchingLocation->get_root() + req.get_request_uri();

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			if (Utils::fileExists(filePath + matchingLocation->get_index()))
				return sendContent(filePath + matchingLocation->get_index()); // Return the index file
			else
			{
				if (matchingLocation->is_autoindex())
				{
					//TODO return directory listing -- is this something we want / need to implement?
				}
				else
				{
					std::string errorPage;
					auto it = matchingServer->get_error_pages().find(403);
					if (it != matchingServer->get_error_pages().end())
						errorPage = it->second;
					else
						errorPage = "www/403.html"; // TODO:Default 403 page
					return sendErrorPage(errorPage);
				}
			}
		}
		return sendContent(filePath); // Return the file content
	}
	else
	{
		std::string errorPage;
		auto it = matchingServer->get_error_pages().find(404);
		if (it != matchingServer->get_error_pages().end())
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

	std::string filePath = matchingLocation->get_root() + req.get_request_uri();
}




