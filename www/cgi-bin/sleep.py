#!/usr/bin/env python3

import time

time.sleep(2)

print("Content-Type: text/html", end="\r\n")
print("", end="\r\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI script output</title></head>")
print("<body>")
print("<h1>C</h1>")
print("<p>Hello, world!</p>")
print("</body>")
print("</html>")