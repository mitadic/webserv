#!/usr/bin/env python3

import time

time.sleep(2)

print("Content-Type: text/html", end="\r\n")
print("\r\n", end="\r\n")  # Body separator
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI script output</title></head>")
print("<body>")
print("<h1>This is my first CGI script</h1>")
print("<p>Hello, world!</p>")
print("</body>")
print("</html>")