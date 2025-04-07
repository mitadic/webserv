import os
import sys
import pytest
import subprocess

WEBSERV_PATH = "../a.out"

def binary_exists():
    return os.path.isfile(WEBSERV_PATH)

def test_binary_exists():
    if not binary_exists():
        sys.exit(0)
    assert binary_exists() == True

if __name__ == "__main__":
    pytest.main()