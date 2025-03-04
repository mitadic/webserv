/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageGenerator.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 20:16:01 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/01 11:46:58 by aarponen         ###   ########.fr       */
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
		<< "  <p>The server encountered an error.</p>\n"
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
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);

	std::string errorPage;
	std::map<int, std::string>::const_iterator it = matchingServer->get_error_pages().find(req.get_response_status());
	if (it != matchingServer->get_error_pages().end())
	{
		errorPage = Utils::readFile(it->second);
	}
	else
	{
		errorPage = generateErrorPage(req.get_response_status());
	}
	std::ostringstream response;
	response << "HTTP/1.1 " << status_messages[req.get_response_status()] << "\r\n"
				<< "Content-Type: text/html\r\n"
				<< "Content-Length: " << errorPage.size() << "\r\n"
				<< "\r\n"
				<< errorPage;
	return response.str();

}