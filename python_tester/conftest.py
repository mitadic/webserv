import pytest
import subprocess
import time
import requests
from pathlib import Path

WEBSERV_PATH = "./a.out"

# the deacorator is used to mark a function as a fixture
# the session scope means the fixture is created once per test session
# any test function that includes webserver will trigger this fixture
@pytest.fixture(scope="session")
def webserver():
	"""Start the webserver for the entire test session"""
	process = subprocess.Popen([WEBSERV_PATH])
	time.sleep(2)  # Give server time to start
	yield process # this is where the test will run
	process.terminate()
	process.wait()

@pytest.fixture(scope="session")
def webserver_quickstart():
	"""Start the webserver for checking bad config files"""
	process = subprocess.run(WEBSERV_PATH, config_file)
	return process.returncode

@pytest.fixture(scope="session")
def base_url():
	"""Base URL for the default server"""
	return "http://127.0.0.1:8080"

@pytest.fixture(scope="session")
def secondary_url():
	return "http://127.0.0.2:9090"
