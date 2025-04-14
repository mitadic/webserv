import os
import sys
import pytest
import subprocess
import requests

WEBSERV_PATH = "./a.out"

# def test_cgi_delete_501(webserver, base_url):
# 	# Does pass once once DELETE is added to /cgi-bin/ in default.conf
# 	assert requests.delete(f"{base_url}/cgi-bin/hello.py").status_code == 501

# def test_post_contact(webserver, secondary_url):
# 	# I need Petra's help and explanation
# 	response = requests.post(f"{secondary_url}/cgi-bin/secondary_contact_form.py/guest-book", data="name: Sophie\r\nmessage: Hello\r\nemail: a@b.com")
# 	assert response.status_code == 201
# 	assert "Form submitted" in response.text

def test_receiving_non_legal_cookies_from_script(webserver, base_url):
	response = requests.get(f"{base_url}/cgi-bin/gibberish_headers.py")
	assert response.status_code == 200
	assert "fruit-type" in response.headers

def test_get_correct_script_body_length(webserver, base_url):
	response = requests.get(f"{base_url}/cgi-bin/post_test.py")
	assert response.status_code == 200
	content_length = int(response.headers.get("Content-Length", 0))
	assert content_length == len(response.text)

# def test_timeout_base(webserver, base_url):
# 	response = requests.get(f"{base_url}/cgi-bin/infinite_sleep.py")
# 	assert response.status_code == 504

# def test_timeout_secondary(webserver, secondary_url):
# 	response = requests.get(f"{secondary_url}/cgi-bin/infinite_sleep.py")
# 	assert response.status_code == 504


# curl -X POST http://127.0.0.1:8080/cgi-bin/secondary_upload_form.py \
# -F "file=hello.txt"
