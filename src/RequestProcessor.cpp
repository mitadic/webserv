/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/01 13:44:40 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestProcessor.hpp"
#include "Request.hpp"  // safe in .cpp, won't cause circular imports

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

std::string createContentString(const std::string& file)
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
// - if not, throw 405 error page
// check if the file exists
// if it's a file, return the file content
// if it doesn't exist, return 404 error page
// if it's a directory, show default index file
// - if index file doesn't exist, show directory content if autoindex is on
// - if autoindex is off, return 403 error page

std::string RequestProcessor::processGet(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);
	const Location* matchingLocation = Utils::getLocation(req, matchingServer);

	if (!matchingLocation->is_get())
		throw RequestException(CODE_405);

	std::string filePath = matchingLocation->get_root() + req.get_request_uri();

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			if (Utils::fileExists(filePath + matchingLocation->get_index()))
				return createContentString(filePath + matchingLocation->get_index());
			else
			{
				if (matchingLocation->is_autoindex())
				{
					//TODO return directory listing -- is this something we want / need to implement?
				}
				else
					throw RequestException(CODE_403);
			}
		}
		return createContentString(filePath);
	}
	else
	{
		throw RequestException(CODE_404);;
	}
}

// POST method
// get the server block that corresponds to the request
// get the location that corresponds to the request
// check if the method is allowed in the location
// - if not, throw 405 error page
// 
std::string RequestProcessor::processPost(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	//TDO
	return "HTTP/1.1 201 Placeholder_OK\r\n\r\n";
}

// DELETE method
// get the server block that corresponds to the request
// get the location that corresponds to the request
// check if the method is allowed in the location
// - if not, throw 405 error page
// check if the file exists
// if it's a file, try to delete
// if it doesn't exist, return 404 error page
// if it's a directory, return 403 error page
// if deletion is successful, return 204 No Content
std::string RequestProcessor::processDelete(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);
	const Location* matchingLocation = Utils::getLocation(req, matchingServer);

	if (!matchingLocation->is_del())
		throw RequestException(CODE_405);

	std::string filePath = matchingLocation->get_root() + req.get_request_uri();

	if (!Utils::fileExists(filePath))
		throw RequestException(CODE_404);

	if (Utils::isDirectory(filePath))
		throw RequestException(CODE_403); // TODO: Or do we want to allow directory deletion and implement recursive deletion?

	else if (std::remove(filePath.c_str()) != 0)
			throw RequestException(CODE_500);  // CHECK: "Internal Server Error" best choice here?

	std::ostringstream response;
	response << "HTTP/1.1 204 No Content\r\n"
				<< "\r\n";
	return response.str();
}





