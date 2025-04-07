import os
import sys
import pytest
import subprocess
import requests

WEBSERV_PATH = "./a.out"

def test_cgi_delete_501(webserver, base_url):
	assert requests.delete(f"{base_url}/cgi-bin/hello.py").status_code == 501