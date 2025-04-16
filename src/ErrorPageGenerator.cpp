/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageGenerator.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 20:16:01 by aarponen          #+#    #+#             */
/*   Updated: 2025/04/16 09:50:45 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPageGenerator.hpp"

std::string generateErrorPage(int status)
{
	std::string message = status_messages[status];

	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "  <meta charset=\"UTF-8\">\n"
		<< "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		<< "  <title>" << message << "</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "  <h1>" << message << "</h1>\n"
		<< "  <p>If 4XX, amend. Else if 5XX, potentially try again.</p>\n"
		<< "</body>\n"
		<< "</html>";

	return oss.str();
}

/**
 * @brief Create error page
 * @param req Reference to a request that the ServerEngine is processing
 * @param server_blocks Reference to the vector of ServerBlocks that the ServerEngine is using
 */
std::string ErrorPageGenerator::createErrorPage(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* server = Utils::getServerBlock(req, server_blocks);
	int error = req.get_response_status();
	int http_status_code = status_code_values[error];

	std::ostringstream oss;
	oss << "Creating error page for status code: " << http_status_code;
	Log::log(oss.str(), DEBUG);

	std::string errorPage;

	std::map<int, std::string>::const_iterator it = server->get_error_pages().find(http_status_code);
	// print avilable error pages

	const Location* location = Utils::getLocation(req, server);
	struct stat sb;
	if (location && it != server->get_error_pages().end() && stat(("." + location->get_root() + it->second).c_str(), &sb) != -1)
	{
		std::string errorPagePath = "." + location->get_root() + it->second;
		Log::log("Using custom error page: " + errorPagePath, DEBUG);
		errorPage = Utils::readFile(errorPagePath);
	}
	else
	{
		Log::log("Using default error page", DEBUG);
		errorPage = generateErrorPage(error);
	}
	std::ostringstream response;
	response << "HTTP/1.1 " << status_messages[error] << "\r\n"
				<< "Content-Type: text/html\r\n"
				<< "Content-Length: " << errorPage.size() << "\r\n"
				<< "\r\n"
				<< errorPage;
	return response.str();
}
