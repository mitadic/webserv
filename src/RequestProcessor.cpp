/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/04 11:19:39 by aarponen         ###   ########.fr       */
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

std::string createContentString(const std::string& file, const std::string& mimeType)
{
	std::string body = Utils::readFile(file);

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Type: " << mimeType << "\r\n"
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
// - if it doesn't exist, return 404 error page
// if it's a directory, show default index file
// - if index file doesn't exist, show directory content if autoindex is on
// - if autoindex is off, return 403 error page
// if it's a file, return the file content if it's an accepted type
// - if it's not an accepted type, return 406 error page
std::string RequestProcessor::processGet(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);
	const Location* matchingLocation = Utils::getLocation(req, matchingServer);

	if (!matchingLocation->is_get())
		throw RequestException(CODE_405); // Method Not Allowed

	std::string filePath = matchingLocation->get_root() + req.get_request_uri();

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			if (Utils::fileExists(filePath + matchingLocation->get_index()))
				filePath += matchingLocation->get_index();
			else
			{
				if (matchingLocation->is_autoindex())
				{
					//TODO return directory listing page
				}
				else
					throw RequestException(CODE_403); // Forbidden
			}
		}

		std::string mimeType = defineMime(filePath);
		std::vector<std::string> acceptHeader = req.get_accepted_types();

		if (acceptHeader.size() > 0)
		{
			bool matchFound = false;
			for (std::vector<std::string>::const_iterator it = acceptHeader.begin(); it != acceptHeader.end(); ++it)
{				const std::string& acceptedType = *it;
				if (acceptedType == mimeType || acceptedType == "*/*" ||
					(acceptedType.find("/*") != std::string::npos && acceptedType.substr(0, acceptedType.find("/")) == mimeType.substr(0, mimeType.find("/"))))
				{
					mimeType = acceptedType;
					break;
				}
			}
			if (!matchFound)
				throw RequestException(CODE_406); // Not Acceptable
		}

		return createContentString(filePath, mimeType);
	}
	else
	{
		throw RequestException(CODE_404); // Not Found
	}
}

// POST method
// get the server block that corresponds to the request
// get the location that corresponds to the request
// check if the method is allowed in the location
// - if not, throw 405 error page
// check the content type to determine how to process the request
// - If it’s multipart/form-data, treat it as a file upload (and use appropriate middleware or parsers to extract files).
// - If it’s application/json, parse it as JSON.
// - If it’s application/x-www-form-urlencoded, process it as form data.
// If the request is for uploading a file, make sure uploads are allowed in the location
// - if not, throw error page
std::string RequestProcessor::processPost(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);
	const Location* matchingLocation = Utils::getLocation(req, matchingServer);

	if (!matchingLocation->is_post())
		throw RequestException(CODE_405);

	int contentTypeIdx = req.get_content_type_idx();

	switch (contentTypeIdx)
	{
	case APPLICATION_X_WWW_FORM_URLENCODED: // form submissions
	{
		// Process form data
		break;
	}
	case MULTIPART_FORM_DATA: // file uploads
	{
		// Process file uploads
		break;
	}
	case TEXT_PLAIN:
	case TEXT_HTML:
	case TEXT_XML:
	case APPLICATION_XML:
	case APPLICATION_XHTML_XML:
	case APPLICATION_OCTET_STREAM:
	case IMAGE_GIF:
	case IMAGE_JPEG:
	case IMAGE_PNG:
	{
		std::string body = req.get_request_body();
		// Process the body content
		break;
	}
	default:
		throw RequestException(CODE_415); // Unsupported Media Type
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n" << "\r\n";
	return response.str();
}


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





