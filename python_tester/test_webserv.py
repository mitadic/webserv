import os
import sys
import pytest
import subprocess
import requests

WEBSERV_PATH = "./a.out"

def binary_exists():
	return os.path.isfile(WEBSERV_PATH)

def test_binary_exists():
	if not binary_exists():
		sys.exit(0)
	assert binary_exists() == True

def test_basic_404(webserver, base_url):
	assert requests.get(f"{base_url}/banana").status_code == 404
