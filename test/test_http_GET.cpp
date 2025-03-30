#include <gtest/gtest.h>
#include "test_http_helpers.hpp"

// Test suite for HTTP requests

// --- GET requests ---

// Test GET request - succesful
TEST(HTTPTestsGET, TestGET_success)
{
	long response_code;
	std::string response = performRequest(std::string(ADDRESS) + "/index.html", "GET", "", &response_code, "");

	EXPECT_EQ(response_code, 200);									// Check if the response code is 200
	EXPECT_NE(response.find("<!DOCTYPE html>"), std::string::npos); // Check if HTML content is returned
}

// Test GET request - "/" should default to index.html
TEST(HTTPTestsGET, TestGET_default)
{
	long response_code;

	std::string response = performRequest(std::string(ADDRESS) + "/", "GET", "", &response_code, "");
	EXPECT_EQ(response_code, 200);
	EXPECT_NE(response.find("<!DOCTYPE html>"), std::string::npos); // Check if HTML content is returned
	EXPECT_NE(response.find("<title>Three Socketeers | Home</title>"), std::string::npos); // Check if the title as expected
}

// Test GET request - file not found
TEST(HTTPTestsGET, TestGET_file_not_found)
{
	long response_code;
	std::string response = performRequest(std::string(ADDRESS) + "/nonexistent.html", "GET", "", &response_code, "");

	EXPECT_EQ(response_code, 404); // Check if the response code is 404
	EXPECT_NE(response.find("Not Found"), std::string::npos); // Check if the error message is present
}

// Test GET request - directory listing
TEST(HTTPTestsGET, TestGET_directory_listing)
{
	long response_code;
	std::string response = performRequest(std::string(ADDRESS) + "/uploads/", "GET", "", &response_code, "");

	EXPECT_EQ(response_code, 200); // Check if the response code is 200
	EXPECT_NE(response.find("test.txt"), std::string::npos); // Check if the test file is listed
}

// Test GET request - navigate above root (not allowed)
// Note: using telnet, because browser and cURL both normalize the path
TEST(HTTPTestsGET, TestGET_navigate_above_root)
{
    std::string raw_request = "GET /uploads/../../Makefile HTTP/1.1\r\n"
                              "Host: 127.0.0.2:8080\r\n"
                              "Connection: close\r\n\r\n";

    try {
        std::string response = sendRawHttpRequest(raw_request, "127.0.0.2", 8080);

        // Check if the response contains the expected status code (403 Forbidden)
        EXPECT_NE(response.find("403 Forbidden"), std::string::npos);
    } catch (const std::exception& e) {
        FAIL() << "Exception occurred: " << e.what();
    }
}

// Test GET request - return unaccepted type
TEST(HTTPTestsGET, TestGET_unaccepted_type)
{
    std::string raw_request = "GET /uploads/tunnel.png HTTP/1.1\r\n"
                              "Host: 127.0.0.2:8080\r\n"
							  "Accept: text/html\r\n"
                              "Connection: close\r\n\r\n";

    try {
        std::string response = sendRawHttpRequest(raw_request, "127.0.0.2", 8080);

        // Check if the response contains the expected status code (403 Forbidden)
        EXPECT_NE(response.find("406 Not Acceptable"), std::string::npos);
    } catch (const std::exception& e) {
        FAIL() << "Exception occurred: " << e.what();
    }
}

