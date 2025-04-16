/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_http_helpers.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarponen <aarponen@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/30 15:28:33 by aarponen          #+#    #+#             */
/*   Updated: 2025/03/30 18:48:30 by aarponen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_HTTP_HELPERS_H
# define TEST_HTTP_HELPERS_H
# include <curl/curl.h>
# include <string>
# include <fstream>
# include <sstream>
# include <iostream>
# include <netinet/in.h> // For sockaddr_in
# include <arpa/inet.h>  // For inet_pton
# include <unistd.h>     // For close

# define ADDRESS "http://127.0.0.2:8080"

// Helper function to capture response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

// Helper function to perform HTTP requests
std::string performRequest(const std::string &url, const std::string &method, const std::string &body = "", long *response_code = nullptr, std::string content_type = "")
{
	CURL *curl = curl_easy_init();
	if (!curl)
	{
		throw std::runtime_error("Failed to initialize CURL");
	}

	std::string response_data;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
	curl_slist *headers = nullptr;

	if (method == "POST")
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		std::string content_type_header = "Content-Type: " + content_type;
		headers = curl_slist_append(headers, content_type_header.c_str());
		headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
		headers = curl_slist_append(headers, "Connection: keep-alive");

		// Add Content-Length header
		std::ostringstream content_length;
		content_length << "Content-Length: " << body.size();
		headers = curl_slist_append(headers, content_length.str().c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	CURLcode res = curl_easy_perform(curl);
	if (response_code)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);
	}
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		// Debugging: Log the CURL error
		std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
		throw std::runtime_error(curl_easy_strerror(res));
	}

	return response_data;
}

// Helper function to send raw HTTP requests
std::string sendRawHttpRequest(const std::string& request, const std::string& host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        throw std::runtime_error("Failed to connect to server");
    }

    // Send the raw HTTP request
    send(sock, request.c_str(), request.size(), 0);

    // Receive the response
    char buffer[4096];
    std::string response;
    ssize_t bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response += buffer;
    }

    close(sock);
    return response;
}

#endif // TEST_HTTP_HELPERS_H
