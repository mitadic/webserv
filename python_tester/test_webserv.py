import os
import sys
import pytest
import subprocess
import requests

WEBSERV_PATH = "../a.out"
# THREE_SOCKETEERS = "http://127.0.0.1:8080"
# SECONDARY = "http://127.0.0.2:9090"

def binary_exists():
    return os.path.isfile(WEBSERV_PATH)

def test_binary_exists():
    if not binary_exists():
        sys.exit(0)
    assert binary_exists() == True

def test_binary_launches():
    assert webserver() == 0

def test_basic_get(webserver, base_url):
    assert requests.get(base_url).status_code == 200



if __name__ == "__main__":
    pytest.main()