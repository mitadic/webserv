#!/usr/bin/env python3

import os
import sys
import urllib.parse
import logging
from datetime import datetime

log_dir = "/home/pbencze/Documents/42cursus/Github/Webserv/www/logs"
os.makedirs(log_dir, exist_ok=True)  # Ensure the logs directory exists
log_file = os.path.join(log_dir, "guest_book.log")

if not os.path.exists(log_file):
	with open(log_file, "w") as f:
		pass

# Configure logging to append to the file
logging.basicConfig(filename=log_file, level=logging.INFO, format="%(asctime)s - %(message)s")

# Read form data (POST request)
form_data = sys.stdin.read(int(os.environ.get("CONTENT_LENGTH", 0)))

# Parse the URL-encoded form data
form_params = urllib.parse.parse_qs(form_data)

content_type = "text/html"

# Extract individual form fields
name = form_params.get("name", [""])[0]
email = form_params.get("email", [""])[0]
message = form_params.get("message", [""])[0]

# Log user request with form data
logging.info(f"User Request: Name={name}, Email={email}, Message={message}")


# Check if all fields were filled
if name and email and message:
	# Redirect using HTTP Location header
	print("Status: 302 Found\r\n", end="")
	print("Location: /success.html\r\n", end="")
	# print("Content-Length: 0\r\n", end="")
	print("\r\n", end="")  # End of headers
else:
	# Output the content type
	print(f"Content-Type: {content_type}", end="\r\n")
	print(f"Status: 400 Bad Request", end="\r\n")
	print("\r\n", end="")  # Body separator
	# Basic HTML structure for the response
	print("<html>")
	print("<head><title>Contact Form Submission</title></head>")
	print("<body>")
	print("<h1>Error: Missing information</h1>")
	print("<p>Please ensure all fields are filled out.</p>")
	print("</body>")
	print("</html>")
