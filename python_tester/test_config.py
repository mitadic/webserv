import pytest
import subprocess
import requests

def test_bad_config_files(webserver_quickstart):
    assert webserver_quickstart("python_tester/test_conf/bad/error.conf") != 0
