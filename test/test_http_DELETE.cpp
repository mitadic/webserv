#include <gtest/gtest.h>
#include "test_http_helpers.hpp"

// Test suite for HTTP requests

// --- DELETE ---

// Test DELETE request - success
TEST(HTTPTestsDELETE, TestDELETE_success)
{
	long response_code;

	// Create a temporary file to delete
	std::ofstream temp_file("./www/three-socketeers/uploads/temp_file.txt");
	temp_file << "Temporary file content";
	temp_file.close();

	std::string response = performRequest(std::string(ADDRESS) + "/uploads/temp_file.txt", "DELETE", "", &response_code, "");

	EXPECT_EQ(response_code, 204);														// Check if the response code is 204
	EXPECT_FALSE(std::ifstream("./www/three-socketeers/uploads/temp_file.txt").good()); // Check if the file is deleted
}

// Test DELETE request - file not found
TEST(HTTPTestsDELETE, TestDELETE_nonexisting_file)
{
	long response_code;

	std::string response = performRequest(std::string(ADDRESS) + "/uploads/nothing.txt", "DELETE", "", &response_code, "");
	EXPECT_EQ(response_code, 404); // Check if the response code is 404
	EXPECT_NE(response.find("Not Found"), std::string::npos); // Check if the error message is present
}

// Test DELETE request - directory deletion (not allowed)
TEST(HTTPTestsDELETE, TestDELETE_directory)
{
	long response_code;

	std::string response = performRequest(std::string(ADDRESS) + "/uploads/alise/", "DELETE", "", &response_code, "");
	EXPECT_EQ(response_code, 403); // Check if the response code is 403
	EXPECT_NE(response.find("Forbidden"), std::string::npos); // Check if the error message is present
	EXPECT_TRUE(std::ifstream("./www/three-socketeers/uploads/alise").good()); // Check if the directory is not deleted
}

// Test DELETE request with path traversal (not allowed)
TEST(HTTPTestsDELETE, TestDELETE_path_travelsal)
{
    std::string raw_request = "DELETE /uploads/../index.html HTTP/1.1\r\n"
                              "Host: 127.0.0.1:8080\r\n"
                              "Connection: close\r\n\r\n";

    try {
        std::string response = sendRawHttpRequest(raw_request, "127.0.0.1", 8080);

        // Check if the response contains the expected status code (403 Forbidden)
        EXPECT_NE(response.find("403 Forbidden"), std::string::npos);
    } catch (const std::exception& e) {
        FAIL() << "Exception occurred: " << e.what();
    }
}
