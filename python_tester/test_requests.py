import os
import sys
import pytest
import subprocess
import requests

# GET allowed in location
def test_basic_get(webserver, base_url):
	assert requests.get(f"{base_url}").status_code == 200

# Parameterized test for POST requests
@pytest.mark.parametrize("file_name", [
	("post_test.txt"),
	("boot.jpg"),
	("meow.png"),
	("logo.webp"),
	("ballad.mp3"),
])

# POST allowed in location
# uploading a file
def test_post_file(webserver, base_url, file_name):
	upload_path = "/uploads/"
	file_path = f"./test_files/{file_name}"
	with open(file_path, "rb") as f:
		response = requests.post(f"{base_url}{upload_path}", files={"file": (file_name, f)})
	assert response.status_code == 201, f"Unexpected status code for {file_name}: {response.status_code}"


# POST missing Content-Length header
# CURL equivalent: difficult to emulate
def test_post_contact_form(webserver, base_url):
	response = requests.post(f"{base_url}/contact.html", data={"subject": "Hola", "message": "Hello"}, headers={"Content-Type": "text/plain"})
	assert response.status_code == 411

# POST allowed in location
# CURL equivalent: curl -X POST http://127.0.0.1:8080/contact.html -H "Content-Type: text/plain" -d "subject=Hola&message=Hello"
def test_post_contact_form(webserver, base_url):
	response = requests.post(f"{base_url}/contact.html", data={"subject": "Hola", "message": "Hello"})
	assert response.status_code == 201

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

def test_basic_405(webserver, secondary_url, base_url):
	response = requests.get(f"{secondary_url}/nomethods/")
	assert response.status_code == 405
	response = requests.post(f"{secondary_url}/nomethods/", data="This one needed data added in order for Content-Length to be generated")
	assert response.status_code == 405
	response = requests.delete(f"{secondary_url}/nomethods/")
	assert response.status_code == 405

def test_invalid(webserver, secondary_url, base_url):
	response = requests.request("INVALID", f"{base_url}")
	assert response.status_code == 501 # @petra changed from 405 to 501, makes sense right?

def test_autoindex(secondary_url):
	# Test that autoindex is enabled for /uploads/
	response = requests.get(f"{secondary_url}/uploads/")
	assert response.text.find("Index of /uploads/") != -1
	assert response.status_code == 200
	# Test that autoindex is disabled for /cgi-bin/
	response = requests.get(f"{secondary_url}/cgi-bin/")
	assert response.text.find("Index of /cgi-bin/") == -1
	assert response.status_code == 403

# fail: loading forever
def test_redirect(webserver):
	url = "http://127.0.0.18:7070"

	response = requests.get(f"{url}/tube/", allow_redirects=False)
	assert response.status_code == 308
	assert response.headers["Location"] == "https://youtube.com"

	response = requests.get(f"{url}/intra/", allow_redirects=False)
	assert response.status_code == 301
	assert response.headers["Location"] == "https://intra.42.fr"

	response = requests.get(f"{url}/google/", allow_redirects=False)
	assert response.status_code == 302
	assert response.headers["Location"] == "https://www.google.com"

	response = requests.get(f"{url}/redirect/", allow_redirects=False)
	assert response.status_code == 307
	assert response.headers["Location"] == "/secondary/"

def test_path(webserver, base_url):
	response = requests.get(f"{base_url}/../../../uploads/")
	assert response.status_code == 200 # or 403

def test_locations(webserver, base_url):
	response = requests.get(f"{base_url}/uploads/")
	assert response.status_code == 200
	response = requests.get(f"{base_url}/uploads")
	assert response.status_code == 200

