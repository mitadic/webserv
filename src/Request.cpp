/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:49:24 by aarponen          #+#    #+#             */
/*   Updated: 2025/02/24 19:48:11 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

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


// ------- METHODS --------------

std::string Request::handleMethod()
{
	switch (method)
	{
	case GET:
		return processGet();
	case POST:
		return processPost();
	case DELETE:
		return processDelete();
	default:
		throw std::runtime_error("Unsupported HTTP method");
	}
}

//check if the file exists
// if it does, return the file
// if it doesn't, return 404
// if it's a directory, show default file
std::string Request::processGet()
{
	std::string filePath = "www" + uri;

	if (Utils::fileExists(filePath))
	{
		if (Utils::isDirectory(filePath))
		{
			// TODO: was there something in the config about default files for dir or anything else to consider here?
		}
		return sendContent(filePath, mime_type); // Return the file content
	}
	else {
		return sendErrorPage("www/404.html"); // Return 404 page
	}
}

// write the post data to a file and return the file path to the user (?)
// error page if the file can't be created
std::string Request::processPost()
{
	//TDO
}

// delete the file if it exists and deletions are allowed
// error page if the file can't be deleted
std::string Request::processDelete()
{
	//TODO
}




