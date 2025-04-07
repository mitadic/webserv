import pytest
import subprocess
import time
import requests
from pathlib import Path

WEBSERV_PATH = "../a.out"

@pytest.fixture(scope="session")
def webserver():
	"""Start the webserver for the entire test session"""
	process = subprocess.Popen([WEBSERV_PATH, "-l", "DEBUG", "conf/default.conf"])
	time.sleep(2)  # Give server time to start
	yield process
	process.terminate()
	process.wait()

@pytest.fixture(scope="session")
def base_url():
	"""Base URL for the default server"""
	return "http://127.0.0.1:8080"

@pytest.fixture(scope="session")
def secondary_url():
	return "http://127.0.0.2:9090"