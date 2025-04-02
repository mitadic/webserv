#!/usr/bin/python3

# this is a simple CGI script
# a CGI script must include: content type, blank line, body

print("Content-Type: text/html")
print()  # blank line to separate headers from body
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI script output</title></head>")
print("<body>")
print("<h1>This is my first CGI script</h1>")
print("<p>Hello, world!</p>")
print("</body>")
print("</html>")
