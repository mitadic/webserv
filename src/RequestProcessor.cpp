/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/04/17 14:30:51 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "RequestProcessor.hpp"
#include "Request.hpp" // safe in .cpp, won't cause circular imports
#include "Location.hpp"
#include "ServerBlock.hpp"
#include "CgiHandler.hpp"

// ------- Helper functions ------------

// ------- GET METHOD FUNCTIONS ------------

std::string defineMime(const std::string &file)
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
	else if (extension == "mp3")
		return "audio/mpeg";
	else
		return "application/octet-stream";
}

std::string createContentString(const std::string &file, const std::string &mimeType)
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

void logPageVisit(const Request &req)
{
	if (req.get_request_uri().find(".html") == std::string::npos)
		return;
	std::ofstream logFile("www/logs/page_visits.log", std::ios_base::app);
	std::map<std::string, std::string> cookies = req.get_cookies();
	std::string name;
	if (cookies.find("username") != cookies.end())
		name = cookies.at("username");
	else
		name = "unknown";
	if (logFile.is_open())
	{
		logFile << Utils::generateTimestamp()
				<< " | Session ID: " << std::setw(10) << std::left << req.get_cookies().at("sessionid")
				<< " | Name: " << std::setw(10) << std::left << name
				<< " | Page: " << req.get_request_uri()
				<< std::endl;
		logFile.close();
	}
	else
		std::cerr << "Unable to open log file." << std::endl;
}

// -----------POST METHOD FUNCTIONS ------------

void logFormSubmission(const Request &req, std::map<std::string, std::string> formData)
{
	std::ofstream logFile("www/logs/form_submissions.log", std::ios_base::app);
	std::map<std::string, std::string> cookies = req.get_cookies();
	std::string name;
	if (cookies.find("username") != cookies.end())
		name = cookies.at("username");
	else
		name = "unknown";
	std::string data;
	for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
	{
		data += "\n" + it->first + ": " + it->second;
	}
	if (logFile.is_open())
	{
		logFile << Utils::generateTimestamp()
				<< " | Session ID: " << std::setw(10) << std::left << req.get_cookies().at("sessionid")
				<< " | Name: " << name
				<< " | Form data: " << std::setw(10) << std::left << data
				<< std::endl;
		logFile.close();
	}
	else
		std::cerr << "Unable to open log file." << std::endl;
}


// Parse the form data and return a map of key-value pairs
std::map<std::string, std::string> parseForm(const std::string &form)
{
	std::map<std::string, std::string> formData;
	std::vector<std::string> pairs = Utils::split(form, '&');

	for (std::vector<std::string>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
	{
		std::vector<std::string> pair = Utils::split(*it, '=');
		if (pair.size() == 2)
			formData[Utils::url_decoder(pair[0])] = Utils::url_decoder(pair[1]);
	}

	return formData;
}

void logUpload(const Request &req, std::string original_filename, std::string filename, std::string new_filename, bool success)
{
	std::ofstream logFile("www/logs/files.log", std::ios_base::app);
	std::map<std::string, std::string> cookies = req.get_cookies();
	std::string name;
	if (cookies.find("username") != cookies.end())
		name = cookies.at("username");
	else
		name = "unknown";
	std::string status = success ? "SUCCESS" : "REQUESTED";
	if (logFile.is_open())
	{
		logFile << Utils::generateTimestamp()
				<< " | Session ID: " << std::setw(10) << std::left << req.get_cookies().at("sessionid")
				<< " | Name: " << std::setw(10) << std::left << name
				<< " | UPLOAD"
				<< " | Status: " << std::setw(10) << std::left << status
				<< " | File: " << "original= " << original_filename
				<< ", sanitized= " << filename
				<< ", saved= " << new_filename

				<< std::endl;
		logFile.close();
	}
	else
		std::cerr << "Unable to open log file." << std::endl;
}

// For file uploads:
// - split the request body into parts per boundary
// - for each part, extract the filename and the file content
// - sanitize the filename to prevent directory traversal attacks
// - save the files to the server in the upload directory
// -- if the file can't be saved, throw 500 error page
void parseMultipartFormData(const Request &req, const Location *location)
{
	std::string boundary = "--" + Utils::findBoundary(req);
	Log::log("Boundary: " + boundary, DEBUG);

	std::string request_body_str = req.get_request_body_as_str();
	if (request_body_str.find(boundary) == std::string::npos)
	{
		Log::log("Boundary not found in request body", ERROR);
		throw RequestException(CODE_400); // Bad Request
	}

	std::vector<std::string> parts = Utils::split(request_body_str, boundary);
	std::string uploadDir = "." + location->get_upload_location();

	for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it)
	{
		const std::string& part = *it;
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
				if (filenameEnd == std::string::npos)
				{
					Log::log("Filename end quote not found in part", ERROR);
					throw RequestException(CODE_400); // Bad Request
				}

				std::string original_filename = part.substr(filenameStart, filenameEnd - filenameStart);

				std::string filename = Utils::sanitizeFilename(original_filename);
				size_t filename_end = filename.find_last_of('.');
				std::string new_filename;
				if (filename_end != std::string::npos)
					new_filename = filename.substr(0, filename_end);
				 new_filename += "_" + Utils::generateTimestamp();
				 new_filename += filename.substr(filename_end);

				Log::log("Filename to save: " + new_filename, DEBUG);
				bool success = false;
				logUpload(req, original_filename, filename, new_filename, success);

				size_t contentStart = part.find("\r\n\r\n", filenameEnd);
				if (contentStart != std::string::npos)
				{
					contentStart += 4;
					size_t contentEnd = part.rfind("\r\n");
					std::string fileContent = part.substr(contentStart, contentEnd - contentStart);

					std::ostringstream ss;
					ss << "File content size: " << fileContent.size();
					Log::log(ss.str(), DEBUG);

					std::string filePath = uploadDir + "/" + new_filename;
					std::ofstream file(filePath.c_str(), std::ios::binary);

					Log::log("Saving file to: " + filePath, DEBUG);

					if (file.is_open())
					{
						file.write(fileContent.c_str(), fileContent.size());
						file.close();
					}
					else
					{
						Log::log("Error saving file", ERROR);
						throw RequestException(CODE_500); // Internal Server Error
					}
					Log::log("File saved successfully", INFO);
					success = true;
					logUpload(req, original_filename, filename, new_filename, success);
				}
			}
		}
	}
}

// -----------DELETE METHOD FUNCTIONS ------------

void logDelete(const Request &req, const std::string &filename, bool success)
{
	std::ofstream logFile("www/logs/files.log", std::ios_base::app);
	std::map<std::string, std::string> cookies = req.get_cookies();
	std::string name;
	if (cookies.find("username") != cookies.end())
		name = cookies.at("username");
	else
		name = "unknown";
	std::string status = success ? "SUCCESS" : "REQUESTED";
	if (logFile.is_open())
	{
		logFile << Utils::generateTimestamp()
				<< " | Session ID: " << std::setw(10) << std::left << req.get_cookies().at("sessionid")
				<< " | Name: " << std::setw(10) << std::left << name
				<< " | DELETE"
				<< " | Status: " << std::setw(10) << std::left << status
				<< " | File: " << filename
				<< std::endl;
		logFile.close();
	}
	else
		std::cerr << "Unable to open log file." << std::endl;
}


// ------- RequestProcessor class ------------


// ------- METHODS --------------
// Handle redirection
// - if the location has a redirect directive with a status code, return that status code and Location header
// -- for status codes 301 and 302, check if the method is DELETE and throw 405 error page
// - if the location has no redirect directive, return the result of the method processing
std::string RequestProcessor::handleMethod(const Request &req, const std::vector<ServerBlock> &server_blocks)
{
	const ServerBlock *matchingServer = Utils::getServerBlock(req, server_blocks);
	const Location *matchingLocation = Utils::getLocation(req, matchingServer);

	if (!matchingLocation->get_redirect().second.empty())
	{
		const_cast<Location *>(matchingLocation)->redirects++;
		if (matchingLocation->redirects > MAX_REDIRECTS)
			throw RequestException(CODE_508); // Loop Detected
		Log::log("Redirect directive found", DEBUG);
		int response_code = matchingLocation->get_redirect().first;
		if ((response_code == 301 || response_code == 302) && req.get_method() == DELETE)
			throw RequestException(CODE_405); // Method Not Allowed
		Log::log("Redirecting to: " + matchingLocation->get_redirect().second, WARNING);
		std::ostringstream response;
		response << "HTTP/1.1 "
				 << status_messages[match_code(response_code)] << "\r\n"
				 << "Location: " << matchingLocation->get_redirect().second << "\r\n"
				 << "Content-Length: 0\r\n"
				 << "\r\n";
		Log::log("Response: " + response.str(), DEBUG);
		return response.str();
	}

	if (detect_cgi(req, matchingLocation, req.get_method()))
	{
		Log::log("CGI detected", INFO);
		return (""); // returning empty string if cgi is detected for now
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


// ------------CGI HANDLER------------

/**
 * @brief Detects if the request-URI cotains a CGI-script
 * based on the following syntax:
 * [location path]/[filename].[extension]/[PATH_INFO]?[QUERY_STRING]
 * @details @returns false if the location does not allow cgis or if
 * it does not allow the specific extension; throws an exception if
 * the method is DELETE
 * if the extension is accepted in the location block,
 * creates a cgi object with the necessary parameters to be passed to
 * the cgi handling function and execve, @returns true
 */
bool RequestProcessor::detect_cgi(const Request& req, const Location* location, int method)
{
	if ((!location->is_get() && method == GET)
		|| (!location->is_post() && method == POST)
		|| (!location->is_del() && method == DELETE))
		throw RequestException(CODE_405);
	if (location->get_cgi_extensions().empty())
		return false; // check this case
	else
	{
		std::vector<std::string>::const_iterator it;
		for (it = location->get_cgi_extensions().begin(); it != location->get_cgi_extensions().end(); it++)
		{
			if (req.get_request_uri().find(*it) != std::string::npos)
			{
				// optional:  throw exception on unallowed syntax
				const_cast<Request&>(req).cgi = new CgiHandler(req, *location, method); // -> pass info to the cgi object
				const_cast<Request&>(req).set_cgi_status(EXECUTE);
				if (method == DELETE)
					throw RequestException(CODE_501); //delete not allowed on cgi
				return true;
			}
		}
	}
	return false;

}

// -------------- GET method ------------------------
// get the server block that corresponds to the request
// get the location that corresponds to the request
// check if the method is allowed in the location
// - if not, throw 405 error page
// check if URI is safe (does not traverse above the root directory)
// - if not, throw 403 error page
// check if the file exists
// - if it doesn't exist, return 404 error page
// if it's a directory, show default index file
// - if index file doesn't exist, show directory content if autoindex is on
// - if autoindex is off, return 403 error page
// if it's a file, return the file content if it's an accepted type
// - if it's not an accepted type, return 406 error page
std::string RequestProcessor::processGet(const Request &req, const Location *location)
{
	if (!location->is_get())
		throw RequestException(CODE_405); // Method Not Allowed

	if (!Utils::uriIsSafe(req.get_request_uri()))
		throw RequestException(CODE_403); // Forbidden

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
					if (req.get_request_uri()[req.get_request_uri().size() - 1] != '/') 	// if the request uri does not end with a slash, update uri
					{
						filePath += "/";
						const_cast<Request&>(req).set_request_uri(req.get_request_uri() + "/");
					}
					Log::log("Showing directory listing", INFO);
					std::ostringstream response, body;
					body << "<html><head><title>Index of " << req.get_request_uri() << "</title></head><body>"
							 << "<h1>Index of " << req.get_request_uri() << "</h1>"
							 << "<ul>";

					std::vector<std::string> files = Utils::listDirectory(filePath);
					for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
					{
						if (*it == "." || *it == "..")
							continue;
						body << "<li><a href=\"" << req.get_request_uri() + *it << "\">" << *it << "</a></li>";
					}
					body << "</ul></body></html>";

					response << "HTTP/1.1 200 OK\r\n"
							 << "Content-Type: text/html\r\n"
							 << "Content-Length: " << body.str().length() << "\r\n"
							 << "\r\n"
							 << body.str();
					Log::log("Returning autoindex response:\r\n", DEBUG);
					Log::log(response.str(), DEBUG);
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
				const std::string &acceptedType = *it;
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

		logPageVisit(req);
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
std::string RequestProcessor::processPost(const Request &req, const Location *location)
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
		std::map<std::string, std::string> formData = parseForm(req.get_request_body_as_str());
		for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
		{
			std::ostringstream oss; oss << it->first << ": " << it->second;
			Log::log(oss.str(), SETUP);
		}
		logFormSubmission(req, formData);
		Log::log("Form submission processed successfully", INFO);
		std::string body = "Form submitted successfully";
		response << "HTTP/1.1 201 Created\r\n"
				 << "Content-Type: text/plain" << "\r\n"
				 << "Content-Length: " << body.size() << "\r\n"
				 << "\r\n"
				 << body;
		break;
	}
	case MULTIPART_FORM_DATA: // file uploads
	{
		Log::log("Processing multipart form data upload", INFO);
		if (!location->is_upload_allowed())
			throw RequestException(CODE_405); // Method Not Allowed
		parseMultipartFormData(req, location);
		std::string body = "File uploaded successfully";
		response << "HTTP/1.1 201 Created\r\n"
			<< "Content-Type: text/plain" << "\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n"
			<< body;
		break;
	}
	case TEXT_PLAIN:
	{
		Log::log("Processing plain text", INFO);
		std::string textBody = req.get_request_body_as_str();
		Log::log("Received text: " + textBody, DEBUG);
		std::string body = "Text processed successfully";
		response << "HTTP/1.1 200 OK\r\n"
					<< "Content-Type: text/plain\r\n"
					<< "Content-Length: " << body.size() << "\r\n"
					<< "\r\n"
					<< body;
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
// check if the URI is safe (does not traverse above the root directory)
// - if not, throw 403 error page
// check if the file exists
// - if it doesn't exist, return 404 error page
// if it's a file, try to delete it
// if it doesn't exist, return 404 error page (Not Found)
// if it's a directory, return 403 error page (Forbidden)
// if deletion is successful, return 204 No Content
// if deletion fails, return 500 error page (Internal Server Error)
std::string RequestProcessor::processDelete(const Request &req, const Location *location)
{

	if (!location->is_del())
		throw RequestException(CODE_405);

	if (!Utils::uriIsSafe(req.get_request_uri()))
		throw RequestException(CODE_403);

	std::string filePath = "." + location->get_root() + req.get_request_uri();
	bool success = false;
	logDelete(req, req.get_request_uri(), success);

	Log::log("Attempting to delete file: " + filePath, WARNING);

	if (!Utils::fileExists(filePath))
		throw RequestException(CODE_404);

	if (Utils::isDirectory(filePath))
		throw RequestException(CODE_403);

	else if (std::remove(filePath.c_str()) != 0)
		throw RequestException(CODE_500);

	Log::log("File deleted successfully", INFO);
	success = true;
	logDelete(req, req.get_request_uri(), success);
	std::ostringstream response;
	response << "HTTP/1.1 204 No Content\r\n\r\n";
	return response.str();
}
