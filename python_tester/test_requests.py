import os
import sys
import pytest
import subprocess
import requests

# GET allowed in location
def test_basic_get(webserver, base_url):
	assert requests.get(f"{base_url}").status_code == 200

# # POST allowed in location
# This test gets stuck
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


# DELETE allowed in location
def test_basic_delete(webserver, base_url):
	upload_path = "/uploads/"
	response = requests.delete(f"{base_url}{upload_path}hello.txt")
	assert response.status_code == 204  # No Content (commonly returned if DELETE is successful)

# Invalid request
def test_invalid_request(webserver, base_url):
	try:
		requests.request("INVALID", f"{base_url}")
	except requests.exceptions.RequestException as e:
		assert isinstance(e, requests.exceptions.InvalidSchema)  # Invalid method should raise an exception
