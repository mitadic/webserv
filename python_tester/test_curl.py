import os
import sys
import pytest
import subprocess
import requests

def test_curl_get(webserver, base_url):
    """Test a GET request using cURL."""
    result = subprocess.run(
        ["curl", "-s", "-o", "/dev/null", "-w", "%{http_code}", f"{base_url}/"],
        capture_output=True,
        text=True
    )
    assert result.stdout.strip() == "200"  # Check if the HTTP status code is 200


