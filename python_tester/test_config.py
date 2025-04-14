import os
import pytest
import subprocess
import requests

WEBSERV_PATH = "./a.out"
CONFIG_BAD_BASE = "python_tester/test_conf/bad/"
CONFIG_GOOD_BASE = "python_tester/test_conf/good/"
TIMEOUT = 1

BAD_CONFIG_FILES = [
    "error.conf",
	"same_prefix.conf",
	"asymmetric_scope_symbols1.conf",
	"asymmetric_scope_symbols2.conf",
    "invalid_path1.conf",
    "invalid_path2.conf",
    "invalid_path3.conf",
    "invalid_path4.conf",
    "invalid_path5.conf",
    "invalid_path6.conf",
    "nested_sb.conf",
    "port_too_big.conf",
	"host_too_big.conf",
	"no_port.conf",
	"no_host.conf",
	"no_client_body.conf",
	"bad_extension.txt",
	"empty_location.conf",
]

@pytest.mark.parametrize("config_file", BAD_CONFIG_FILES)
def test_bad_config_files(config_file):
    """Test that server exits with non-zero code for bad config files"""
    full_path = os.path.join(CONFIG_BAD_BASE, config_file)

    try:
        # Redirect stdout and stderr to DEVNULL to suppress all output
        process = subprocess.run(
            [WEBSERV_PATH, full_path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=TIMEOUT
        )

        # Process completed within timeout - should have a non-zero return code
        assert process.returncode != 0, \
            f"Config {config_file} should have failed but exited with code {process.returncode}"

    except subprocess.TimeoutExpired:
        # If we reach the timeout, the process didn't exit as expected
        pytest.fail(f"Config {config_file} launched successfully when it should have failed")
        # Attempt cleanup silently
        try:
            subprocess.run(["pkill", "-f", WEBSERV_PATH],
                           stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)
        except:
            pass
