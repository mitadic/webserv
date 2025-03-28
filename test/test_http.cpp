#include <gtest/gtest.h>
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Helper function to capture response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

// Helper function to perform HTTP requests
std::string performRequest(const std::string &url, const std::string &method, const std::string &body = "", long *response_code = nullptr)
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

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	// Debugging: Log the request details
	std::cerr << "Performing " << method << " request to " << url << std::endl;
	if (!body.empty())
	{
		std::cerr << "Request body: " << body << std::endl;
	}

	if (method == "POST")
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

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

// Test GET request
TEST(HTTPTests, TestGET)
{
	long response_code;
	std::string response = performRequest("http://127.0.0.2:8080/index.html", "GET", "", &response_code);

	EXPECT_EQ(response_code, 200);									// Check if the response code is 200
	EXPECT_NE(response.find("<!DOCTYPE html>"), std::string::npos); // Check if HTML content is returned
}

// Test DELETE request
TEST(HTTPTests, TestDELETE)
{
	long response_code;

	// Create a temporary file to delete
	std::ofstream temp_file("./www/three-socketeers/uploads/temp_file.txt");
	temp_file << "Temporary file content";
	temp_file.close();

	std::string response = performRequest("http://127.0.0.2:8080/temp_file.txt", "DELETE", "", &response_code);

	EXPECT_EQ(response_code, 204);														// Check if the response code is 204
	EXPECT_FALSE(std::ifstream("./www/three-socketeers/uploads/temp_file.txt").good()); // Check if the file is deleted
}

// Test POST request
TEST(HTTPTests, TestPOST)
{
	long response_code;
	std::string body = "subject=Test&message=This+is+a+test+message";

	// Add Content-Type header
	std::string response = performRequest("http://127.0.0.2:8080/contact.html", "POST", body, &response_code);

	EXPECT_EQ(response_code, 201);												// Check if the response code is 201
	EXPECT_NE(response.find("Form submitted successfully"), std::string::npos); // Check success message
}
