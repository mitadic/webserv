import os
import sys
import pytest
import subprocess
import requests

# GET allowed
def test_basic_get(webserver, base_url):
	assert requests.get(f"{base_url}").status_code == 200

# POST allowed
def test_basic_post(webserver, base_url):
	response = requests.post(f"{base_url}/uploads/", data={"file": "test.txt", "content": "Hello, World!"})
	assert response.status_code == 201  # Created (commonly returned if POST is successful)

# DELETE allowed
def test_basic_delete(webserver, base_url):
	response = requests.delete(f"{base_url}/uploads/test.txt")
	assert response.status_code == 204  # No Content (commonly returned if DELETE is successful)

# Invalid request
def test_invalid_request(webserver, base_url):
	try:
		requests.request("INVALID", f"{base_url}")
	except requests.exceptions.RequestException as e:
		assert isinstance(e, requests.exceptions.InvalidSchema)  # Invalid method should raise an exception
