/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/05 16:05:11 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestProcessor.hpp"
#include "Request.hpp"  // safe in .cpp, won't cause circular imports

// ------- Helper functions ------------

// ------- GET METHOD FUNCTIONS ------------

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

// -----------POST METHOD FUNCTIONS ------------

// Parse the form data and return a map of key-value pairs
std::map<std::string, std::string> parseForm(const std::string& form)
{
	std::map<std::string, std::string> formData;
	std::vector<std::string> pairs = Utils::split(form, '&');

	for (std::vector<std::string>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
	{
		std::vector<std::string> pair = Utils::split(*it, '=');
		if (pair.size() == 2)
			formData[pair[0]] = pair[1];
	}

	return formData;
}

// For file uploads:
// - split the request body into parts per boundary
// - for each part, extract the filename and the file content
// - sanitize the filename to prevent directory traversal attacks
// - save the files to the server in the upload directory
// -- if the file can't be saved, throw 500 error page
void parseMultipartFormData(const Request& req, const Location* location)
{
	std::string boundary = "--"; // TODO: + req.get_boundary()
	std::vector<std::string> parts = Utils::split(req.get_request_body(), boundary);

	std::string uploadDir = location->get_upload_location();

	for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it)
	{
		std::string part = *it;
		if (part.empty() || part == "--\r\n")
			continue;
		size_t dispositionStart = part.find("Content-Disposition: form-data;");
		if (dispositionStart != std::string::npos)
		{
			size_t filenameStart = part.find("filename=\"", dispositionStart);
			if (filenameStart != std::string::npos)
			{
				filenameStart += 10;
				size_t filenameEnd = part.find("\"", filenameStart);
				std::string filename = part.substr(filenameStart, filenameEnd - filenameStart);

				filename = Utils::sanitizeFilename(filename);

				size_t contentStart = part.find("\r\n\r\n", filenameEnd);
				if (contentStart != std::string::npos)
				{
					contentStart += 4;
					size_t contentEnd = part.rfind("\r\n");
					std::string fileContent = part.substr(contentStart, contentEnd - contentStart);

					std::string filePath = uploadDir + "/" + filename;
					std::ofstream file(filePath.c_str(), std::ios::binary);
					if (file.is_open())
					{
						file << fileContent;
						file.close();
					}
					else
					{
						throw RequestException(CODE_500); // Internal Server Error
					}
				}
			}
		}
	}
}

// ------- METHODS --------------
// Handle redicetion
// - if the location has a redirect directive with a status code, return that status code and Location header
// -- for status codes 301 and 302, check if the method is DELETE and throw 405 error page
// - if the location has no redirect directive, return the result of the method processing
std::string RequestProcessor::handleMethod(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);
	const Location* matchingLocation = Utils::getLocation(req, matchingServer);

	if (!matchingLocation->get_redirect().second.empty())
	{
		if ((matchingLocation->get_redirect().first == 301 || matchingLocation->get_redirect().first == 302)
			&& req.get_method() == DELETE)
			throw RequestException(CODE_405); // Method Not Allowed
		std::ostringstream response;
		response << "HTTP/1.1 " << matchingLocation->get_redirect().first << " "
					<< status_messages[matchingLocation->get_redirect().first] << "\r\n"
					<< "Location: " << matchingLocation->get_redirect().second << "\r\n"
					<< "\r\n";
		return response.str();
	}

	switch (req.get_method())
	{
		case GET:
			return processGet(req, matchingLocation);
		case POST:
			return processPost(req, matchingLocation);
		case DELETE:
			return processDelete(req, matchingLocation);
		default:
			throw RequestException(CODE_405); // Method Not Allowed
	}
}

// -------------- GET method ------------------------
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
std::string RequestProcessor::processGet(const Request& req, const Location* location)
{

	if (!location->is_get())
		throw RequestException(CODE_405); // Method Not Allowed

	std::string filePath = location->get_root() + req.get_request_uri();

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			if (Utils::fileExists(filePath + location->get_index()))
				filePath += location->get_index();
			else
			{
				if (location->is_autoindex())
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

// -------------- POST method -------------------------
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
std::string RequestProcessor::processPost(const Request& req, const Location* location)
{

	if (!location->is_post())
		throw RequestException(CODE_405); // Method Not Allowed

	int contentTypeIdx = req.get_content_type_idx();

	std::ostringstream response;

	switch (contentTypeIdx)
	{
	case APPLICATION_X_WWW_FORM_URLENCODED: // form submissions
	{
		std::map<std::string, std::string> formData = parseForm(req.get_request_body());
		// CHECK:: Log the form data content in the server console:
		for (const auto& pair : formData)
			std::cout << "Form field: " << pair.first << " = " << pair.second << std::endl;
		response << "HTTP/1.1 200 OK\r\n\r\nForm submission processed successfully.";
		break;
	}
	case MULTIPART_FORM_DATA: // file uploads
	{
		if (!location->is_upload_allowed())
			throw RequestException(CODE_405); // Method Not Allowed
		parseMultipartFormData(req, location);
		response << "HTTP/1.1 200 OK\r\n\r\nFile upload processed successfully.";
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
		std::string body = req.get_request_body(); // TODO: Do something with the body? Save as file?
		response << "HTTP/1.1 200 OK\r\n\r\nRequest body processed successfully.";
		break;
	}
	default:
		throw RequestException(CODE_415); // Unsupported Media Type
	}

	return response.str();
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
std::string RequestProcessor::processDelete(const Request& req, const Location* location)
{

	if (!location->is_del())
		throw RequestException(CODE_405);

	std::string filePath = location->get_root() + req.get_request_uri();

	if (!Utils::fileExists(filePath))
		throw RequestException(CODE_404);

	if (Utils::isDirectory(filePath))
		throw RequestException(CODE_403); // TODO: Or do we want to allow directory deletion and implement recursive deletion?

	else if (std::remove(filePath.c_str()) != 0)
			throw RequestException(CODE_500);  // CHECK: "Internal Server Error" best choice here?

	std::ostringstream response;
	response << "HTTP/1.1 204 No Content\r\n\r\n";
	return response.str();
}





