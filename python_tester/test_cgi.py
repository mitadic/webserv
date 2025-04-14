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
	assert response.status_code == 200 # @Milos: changed this to 200 instead of 201, maybe it should be 201
	assert response.headers["Location"] == "/success.html"

# @Milos: added this test, maybe the response should return 400
# if the client does not fill out all fields?
def test_post_contact_incomplete(webserver, secondary_url):
	response = requests.post(f"{secondary_url}/cgi-bin/secondary_contact_form.py/guest-book", data={"name": "Sophie", "message": "Hello"})
	assert response.status_code == 400 # @Milos I guess it would be nice to read the Status header from the cgi output
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

# curl -X POST http://127.0.0.1:8080/cgi-bin/secondary_upload_form.py \
# -F "file=hello.txt"
