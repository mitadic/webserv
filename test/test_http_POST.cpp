#include <gtest/gtest.h>
#include "test_http_helpers.hpp"
#include <dirent.h>

// Test suite for HTTP requests

// --- POST ---

// Test POST request - contact form submission
TEST(HTTPTestsPOST, TestPOST_contact_form_success)
{
	long response_code;
	std::string body = "subject=Alise&message=moikka";

	// Add Content-Type header
	std::string response = performRequest("http://127.0.0.1:8080/contact.html", "POST", body, &response_code, "application/x-www-form-urlencoded");

	EXPECT_EQ(response_code, 201);												// Check if the response code is 201
	EXPECT_NE(response.find("Form submitted successfully"), std::string::npos); // Check success message
}

// Test POST request - file upload
TEST(HTTPTestsPOST, TestPOST_upload_txt)
{
	long response_code;

	std::string file_path = "test/test_files/greetings.txt";

	// Calculate the size of the file
	std::ifstream file_stream(file_path, std::ios::binary);
	if (!file_stream.is_open())
	{
		FAIL() << "Failed to open file: " << file_path;
	}

	// Read the file content into a string
	std::ostringstream file_content_stream;
	file_content_stream << file_stream.rdbuf();
	std::string file_content = file_content_stream.str();
	file_stream.close();

	// Construct the multipart/form-data body
	std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
	std::ostringstream body_stream;
	body_stream << "--" << boundary << "\r\n";
	body_stream << "Content-Disposition: form-data; name=\"file\"; filename=\"greetings.txt\"\r\n";
	body_stream << "Content-Type: text/plain\r\n\r\n";
	body_stream << file_content << "\r\n";
	body_stream << "--" << boundary << "--\r\n";

	std::string body = body_stream.str();

	// Set the Content-Type header for multipart/form-data
	std::string response = performRequest(
		std::string(ADDRESS) + "/upload.html", "POST", body, &response_code, "multipart/form-data; boundary=" + boundary);

	// Check the response
	EXPECT_EQ(response_code, 201); // Check if the response code is 201 (Created)

	// Search for the first file in the uploads directory that starts with "greetings"
	std::string uploaded_file_path;
	DIR *dir = opendir("www/three-socketeers/uploads");
	if (dir == nullptr)
	{
		FAIL() << "Failed to open uploads directory.";
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != nullptr)
	{
		std::string filename = entry->d_name;
		if (entry->d_type == DT_REG && filename.find("greetings") == 0) // Check if it's a regular file and starts with "greetings"
		{
			uploaded_file_path = "www/three-socketeers/uploads/" + filename;
			break;
		}
	}
	closedir(dir);

	// Ensure the uploaded file was found
	if (uploaded_file_path.empty())
	{
		FAIL() << "Failed to find uploaded file with prefix 'greetings' in directory: uploads";
	}

	// Open the uploaded file
	std::ifstream uploaded_file_stream(uploaded_file_path, std::ios::binary);
	if (!uploaded_file_stream.is_open())
	{
		FAIL() << "Failed to open uploaded file: " << uploaded_file_path;
	}

	// Read the uploaded file content into a string
	std::ostringstream uploaded_file_content_stream;
	uploaded_file_content_stream << uploaded_file_stream.rdbuf();
	std::string uploaded_file_content = uploaded_file_content_stream.str();
	uploaded_file_stream.close();

	// Compare the original file content with the uploaded file content
	EXPECT_EQ(file_content, uploaded_file_content) << "Uploaded file content does not match the original file content.";

	// Clean up the uploaded file
	if (remove(uploaded_file_path.c_str()) != 0)
	{
		FAIL() << "Failed to delete uploaded file: " << uploaded_file_path;
	}

}