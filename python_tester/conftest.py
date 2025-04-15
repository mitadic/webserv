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
	valgrind_log = "python_tester/valgrind.log"
	valgrind_command = [
		"valgrind",
		"--leak-check=full",
		"--track-fds=yes",
		"--log-file=" + valgrind_log,
		WEBSERV_PATH
	]
	# process = subprocess.Popen([WEBSERV_PATH]) to run without valgrind
	process = subprocess.Popen(valgrind_command) # to run with valgrind
	time.sleep(2)  # Give server time to start
	yield process # this is where the test will run
	process.terminate() # terminate the process
	process.wait() # wait for it to finish

	# Check the Valgrind log for leaks or errors
	with open(valgrind_log, "r") as log_file:
		valgrind_output = log_file.read()
		print("\n[Valgrind Output]\n")
		print(valgrind_output)

		# Optionally, fail the test session if Valgrind detects issues
		if "definitely lost: 0 bytes" not in valgrind_output or "All heap blocks were freed" not in valgrind_output:
			pytest.fail("Valgrind detected memory leaks or file descriptor issues. Check valgrind.log for details.")

# @pytest.fixture(scope="session")
# def webserver_quickstart():
# 	"""Start the webserver for checking bad config files"""
# 	process = subprocess.run(WEBSERV_PATH, config_file)
# 	return process.returncode

@pytest.fixture(scope="session")
def base_url():
	"""Base URL for the default server"""
	return "http://127.0.0.1:8080"

@pytest.fixture(scope="session")
def secondary_url():
	"""Base URL for the secondary server"""
	return "http://127.0.0.2:9090"
