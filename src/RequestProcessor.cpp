/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/15 18:48:34 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestProcessor.hpp"
#include "Request.hpp"  // safe in .cpp, won't cause circular imports
#include "Location.hpp"
#include "ServerBlock.hpp"

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

	//TODO: Add URL Decoder?

	for (std::vector<std::string>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
	{
		std::vector<std::string> pair = Utils::split(*it, '=');
		if (pair.size() == 2)
			formData[pair[0]] = pair[1];
	}

	return formData;
}

// Find boundary from the request params
std::string findBoundary(const Request& req)
{
	std::vector<std::string> contentTypeParams = req.get_content_type_params();
	for (std::vector<std::string>::const_iterator it = contentTypeParams.begin(); it != contentTypeParams.end(); ++it)
	{
		if (it->find("boundary=") != std::string::npos)
			return it->substr(it->find("boundary=") + 9);
	}
	throw RequestException(CODE_400); // Bad Request
}

// For file uploads:
// - split the request body into parts per boundary
// - for each part, extract the filename and the file content
// - sanitize the filename to prevent directory traversal attacks
// - save the files to the server in the upload directory
// -- if the file can't be saved, throw 500 error page
void parseMultipartFormData(const Request& req, const Location* location)
{
	std::string boundary = "--" + findBoundary(req);
	Log::log("Boundary: " + boundary, DEBUG);

	std::vector<std::string> parts = Utils::split(req.get_request_body(), boundary);

	std::string uploadDir = "." + location->get_upload_location();

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

				Log::log("Filename to save: " + filename, DEBUG);

				size_t contentStart = part.find("\r\n\r\n", filenameEnd);
				if (contentStart != std::string::npos)
				{
					contentStart += 4;
					size_t contentEnd = part.rfind("\r\n");
					std::string fileContent = part.substr(contentStart, contentEnd - contentStart);

					Log::log("File content: " + fileContent, DEBUG);

					std::string filePath = uploadDir + "/" + filename;
					std::ofstream file(filePath.c_str(), std::ios::binary);

					Log::log("Saving file to: " + filePath, DEBUG);

					if (file.is_open())
					{
						file << fileContent;
						file.close();
					}
					else
					{
						throw RequestException(CODE_500); // Internal Server Error
					}
					Log::log("File saved successfully", INFO);
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
		// the browser or Postman handle redirection loops, no need to check manually
		Log::log("Redirect directive found", DEBUG);
		int response_code = matchingLocation->get_redirect().first;
		if ((response_code == 301 || response_code == 302)
			&& req.get_method() == DELETE)
			throw RequestException(CODE_405); // Method Not Allowed
		Log::log("Redirecting to: " + matchingLocation->get_redirect().second, WARNING);
		std::ostringstream response;
		response << "HTTP/1.1 "
					<< status_messages[match_code(response_code)] << "\r\n"
					<< "Location: " << matchingLocation->get_redirect().second << "\r\n"
					<< "\r\n";
		Log::log("Response: " + response.str(), DEBUG);
		return response.str();
	}

	switch (req.get_method())
	{
		case GET:
		{
			Log::log("GET request: " + req.get_request_uri(), INFO);
			return processGet(req, matchingLocation);
		}
		case POST:
		{
			Log::log("POST request: " + req.get_request_uri(), INFO);
			return processPost(req, matchingLocation);
		}
		case DELETE:
		{
			Log::log("DELETE request: " + req.get_request_uri(), INFO);
			return processDelete(req, matchingLocation);
		}
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

	std::string filePath = "." + location->get_root() + req.get_request_uri();

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			if (Utils::fileExists(filePath + "/index.html"))
				filePath += "/index.html";
			else
			{
				if (location->is_autoindex()) // Show directory listing
				{
					Log::log("Showing directory listing", INFO);
					std::ostringstream response;
					response << "HTTP/1.1 200 OK\r\n"
								<< "Content-Type: text/html\r\n"
								<< "\r\n"
								<< "<html><head><title>Index of " << req.get_request_uri() << "</title></head><body>"
								<< "<h1>Index of " << req.get_request_uri() << "</h1>"
								<< "<ul>";

					std::vector<std::string> files = Utils::listDirectory(filePath);
					for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
					{
						if (*it == "." || *it == "..")
							continue;
						response << "<li><a href=\"" << req.get_request_uri() + "/" + *it << "\">" << *it << "</a></li>";
					}
					response << "</ul></body></html>";
					return response.str();
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
			{
				const std::string& acceptedType = *it;
				if (acceptedType == mimeType || acceptedType == "*/*" ||
					(acceptedType.find("/*") != std::string::npos && acceptedType.substr(0, acceptedType.find("/")) == mimeType.substr(0, mimeType.find("/"))))
				{
					matchFound = true;
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

	std::ostringstream oss;
	oss << "Content type: " << content_types[contentTypeIdx];
	Log::log(oss.str(), DEBUG);

	switch (contentTypeIdx)
	{
	case APPLICATION_X_WWW_FORM_URLENCODED: // form submissions
	{
		Log::log("Processing form submission", INFO);
		std::map<std::string, std::string> formData = parseForm(req.get_request_body());
		std::cout << "New message from " << Utils::url_decoder(formData["name"]) << " =\n" << Utils::url_decoder(formData["message"]) << std::endl;
		Log::log("Form submission processed successfully", INFO);
		std::string body = "form submitted successfully";
		response << "HTTP/1.1 201 OK\r\n"
			<< "Content-Type: text/plain" << "\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n"
			<< body;
		break;
	}
	case MULTIPART_FORM_DATA: // file uploads
	{
		Log::log("Processing file upload", INFO);
		if (!location->is_upload_allowed())
			throw RequestException(CODE_405); // Method Not Allowed
		parseMultipartFormData(req, location);
		std::string body = "file uploaded successfully";
		response << "HTTP/1.1 201 OK\r\n"
			<< "Content-Type: text/plain" << "\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n"
			<< body;
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
// if it doesn't exist, return 404 error page (Not Found)
// if it's a directory, return 403 error page (Forbidden)
// if deletion is successful, return 204 No Content
// if deletion fails, return 500 error page (Internal Server Error)
std::string RequestProcessor::processDelete(const Request& req, const Location* location)
{

	if (!location->is_del())
		throw RequestException(CODE_405);

	std::string filePath = "." + location->get_upload_location() + req.get_request_uri();

	Log::log("Attempting to deleting file: " + filePath, WARNING);

	if (!Utils::fileExists(filePath))
		throw RequestException(CODE_404);

	if (Utils::isDirectory(filePath))
		throw RequestException(CODE_403); // TODO: Or do we want to allow directory deletion and implement recursive deletion?

	else if (std::remove(filePath.c_str()) != 0)
			throw RequestException(CODE_500);

	Log::log("File deleted successfully", INFO);
	std::ostringstream response;
	response << "HTTP/1.1 204 No Content\r\n\r\n";
	return response.str();
}





