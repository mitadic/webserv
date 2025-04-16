import os
import sys
import pytest
import subprocess
import requests

WEBSERV_PATH = "./a.out"

def test_cgi_delete_501(webserver, base_url):
	# Does pass once once DELETE is added to /cgi-bin/ in default.conf
	assert requests.delete("http://127.0.0.18:7070/cgi-bin/hello.py").status_code == 501

def test_post_contact(webserver, secondary_url):
	response = requests.post(f"{secondary_url}/cgi-bin/secondary_contact_form.py/guest-book", data={"name": "Sophie", "message": "Hello", "email": "a@b.com"}, allow_redirects=False)
	assert response.status_code == 200
	assert response.headers["Location"] == "/success.html"

def test_post_contact_incomplete(webserver, secondary_url):
	response = requests.post(f"{secondary_url}/cgi-bin/secondary_contact_form.py/guest-book", data={"name": "Sophie", "message": "Hello"})
	assert response.status_code == 400
	assert "Error: Missing information" in response.text

def test_receiving_non_legal_cookies_from_script(webserver, base_url):
	response = requests.get(f"{base_url}/cgi-bin/gibberish_headers.py")
	assert response.status_code == 200
	assert "fruit-type" in response.headers

def test_get_correct_script_body_length(webserver, base_url):
	response = requests.get(f"{base_url}/cgi-bin/post_test.py")
	assert response.status_code == 200
	content_length = int(response.headers.get("Content-Length", 0))
	assert content_length == len(response.text)

def test_timeout_base(webserver, base_url):
	response = requests.get(f"{base_url}/cgi-bin/infinite_sleep.py")
	assert response.status_code == 504

def test_timeout_secondary(webserver, secondary_url):
	response = requests.get(f"{secondary_url}/cgi-bin/infinite_sleep.py")
	assert response.status_code == 504

def test_query_string(webserver, secondary_url):
	response = requests.get(f"{secondary_url}/cgi-bin/hello_query.py?first_name=Milos&last_name=Tadic")
	assert response.status_code == 200
	assert "Hello, Milos Tadic!" in response.text

def test_php(webserver, secondary_url):
	response = requests.get(f"{secondary_url}/cgi-bin/calendar.php")
	assert response.status_code == 200
	assert "Calendar for" in response.text
	response = requests.get(f"{secondary_url}/cgi-bin/simple_html.php")
	assert response.status_code == 200
	assert "PHP CGI Script Output" in response.text

def test_hello_world(webserver, secondary_url):
	response = requests.get(f"{secondary_url}/cgi-bin/hello_world.py")
	assert response.status_code == 200
	assert "Hello, world!" in response.text

def test_cgi_upload(webserver, secondary_url):
	file_path = f"./test_files/boot.jpg"
	with open(file_path, "rb") as f:
		response = requests.post(f"{secondary_url}/cgi-bin/secondary_upload_form.py", files={"file": ("boot.jpg", f)})
	assert response.status_code == 200, f"Unexpected status code for boot.jpg: {response.status_code}"
	assert "File Upload" in response.text, "Response body does not contain expected content."

def test_bad_status(webserver, secondary_url):
	response = requests.get(f"{secondary_url}/cgi-bin/bad_status.py")
	assert response.status_code == 500
	assert "hello" not in response.text

def test_redirect(webserver, secondary_url):
	response = requests.get(f"{secondary_url}/cgi-bin/redirect.py", allow_redirects=False)
	assert response.status_code == 302 # @Milos
	assert "Location" in response.headers
	assert response.headers["Location"] == "./hello_world.py"
