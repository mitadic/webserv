/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPageGenerator.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 20:16:01 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/28 20:35:13 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPageGenerator.hpp"


std::string errorResponse(const std::string& file, const int status)
{
	std::string body = Utils::readFile(file);

	std::ostringstream response;
	response << "HTTP/1.1 << " << status << " error definition\r\n" //TODO: error definition
				<< "Content-Type: text/html\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "\r\n"
				<< body;
	return response.str();
}

std::string ErrorPageGenerator::createErrorPage(const Request& req, const std::vector<ServerBlock>& server_blocks)
{
	const ServerBlock* matchingServer = Utils::getServerBlock(req, server_blocks);

	std::string errorPage;
	auto it = matchingServer->get_error_pages().find(req.get_response_status());
	if (it != matchingServer->get_error_pages().end())
		errorPage = it->second;
	else
		errorPage = "www/error.html"; // TODO:Default error page
	return errorResponse(errorPage, req.get_response_status());
}