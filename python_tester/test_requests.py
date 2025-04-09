import os
import sys
import pytest
import subprocess
import requests

# GET allowed in location
def test_basic_get(webserver, base_url):
	assert requests.get(f"{base_url}").status_code == 200

# # POST allowed in location
# # This test gets stuck
# def test_basic_post(webserver, base_url):
# 	upload_path = "/uploads/"
# 	response = requests.post(f"{base_url}{upload_path}",
# 		data={"file": "hello.txt"},
# 		headers={"Content-Type": "text/plain"})
# 	assert response.status_code == 201  # Created (commonly returned if POST is successful)

# POST allowed in location
# This test gets stuck
# CURL equivalent: curl -X POST http://127.0.0.1:8080/contact.html -H "Content-Type: text/plain" -d "subject=Hola&message=Hello"
# def test_post_contact_form(webserver, base_url):
# 	response = requests.post(f"{base_url}/contact.html", data={"subject": "Hola", "message": "Hello"}, headers={"Content-Type": "text/plain"})
# 	assert response.status_code == 201
# 	assert "Form submitted" in response.text

def test_basic_delete(webserver, base_url):
	# Define the file path
	upload_path = "/uploads/"
	file_name = "hello.txt"
	file_url = f"{base_url}{upload_path}{file_name}"
	local_file_path = f"./www/three-socketeers/uploads/{file_name}"
	# Step 1: Create the file on the server
	with open(local_file_path, "w") as f:
		f.write("This is a test file for DELETE request.")
	# Step 2: Send the DELETE request
	response = requests.delete(file_url)
	assert response.status_code == 204, f"Unexpected status code: {response.status_code}"
	# Step 3: Verify the file no longer exists
	assert not os.path.exists(local_file_path), "File was not deleted."

def test_delete_inexistent_file(webserver, base_url):
	upload_path = "/uploads/"
	response = requests.delete(f"{base_url}{upload_path}inexistent.txt")
	assert response.status_code == 404, f"Unexpected status code: {response.status_code}"

# Invalid request
def test_invalid_request(webserver, base_url):
	response = requests.request("INVALID", f"{base_url}")
	assert response.status_code == 405

def test_not_allowed_methods(webserver, secondary_url):
	# Test with a method that is not allowed
	response = requests.get(f"{secondary_url}/nomethods/")
	assert response.status_code == 405
	response = requests.post(f"{secondary_url}/nomethods/")
	assert response.status_code == 405
	response = requests.delete(f"{secondary_url}/nomethods/")
	assert response.status_code == 405

