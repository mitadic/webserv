#!/usr/bin/env python3

import os
import sys
import urllib.parse
import logging
from datetime import datetime

# Initialize the logging module (to log to a file)
log_dir = os.environ.get("PATH_TRANSLATED")
os.makedirs(log_dir, exist_ok=True)  # Create 'logs' directory if it doesn't exist

log_file = os.path.join(log_dir, "guest_book.log")

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

# Output the content type
print(f"Content-Type: {content_type}", end="\r\n")
print("\r\n", end="")  # Body separator
# Basic HTML structure for the response
print("<html>")
print("<head><title>Contact Form Submission</title></head>")
print("<body>")
# Check if all fields were filled
if name and email and message:
	print('<meta http-equiv="refresh" content="0;url=/success.html">')  # Redirect using meta refresh
else:
    print("<h1>Error: Missing information</h1>")
    print("<p>Please ensure all fields are filled out.</p>")
print("</body>")
print("</html>")
