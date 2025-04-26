#!/usr/bin/env python3

# from signal import signal, SIGPIPE, SIG_DFL
# signal(SIGPIPE, SIG_DFL)


# this is a simple CGI script
# a CGI script must include: content type, blank line, body

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
