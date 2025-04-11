import os
import sys
import pytest
import subprocess
import requests

WEBSERV_PATH = "./a.out"

def test_cgi_delete_501(webserver, base_url):
	assert requests.delete(f"{base_url}/cgi-bin/hello.py").status_code == 501

def test_post_contact(webserver, secondary_url):
	response = requests.post(f"{secondary_url}/cgi-bin/secondary_contact_form.py/guest-book", data={"name": "Sophie", "message": "Hello", "email:": "a@b.com" }, headers={"Content-Type": "text/plain"})
	assert response.status_code == 201
	assert "Form submitted" in response.text

# curl -X POST http://127.0.0.1:8080/cgi-bin/secondary_upload_form.py \
# -F "file=hello.txt"
