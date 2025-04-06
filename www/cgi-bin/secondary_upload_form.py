#!/usr/bin/env python3

import os
import sys
import cgi

UPLOAD_DIR = "www/secondary/uploads/" #os.environ.get("PATH_INFO", "")
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Read form data using cgi.FieldStorage()
form = cgi.FieldStorage()

# Get the uploaded file
file_item = form["file"]

print(f"Content-Type: text/html", end="\r\n")
print("\r\n", end="")  # Body separator
print("<html>")
print("<head><title>File Upload</title></head>")
print("<body>")
# Check if a file was uploaded
if file_item.filename:
    filename = os.path.basename(file_item.filename)  # Prevent directory traversal
    file_path = os.path.join(UPLOAD_DIR, filename)

    # Save the file
    with open(file_path, "wb") as f:
        f.write(file_item.file.read())

    print(f"<h1>File '{filename}' uploaded successfully!</h1>")
    print('<meta http-equiv="refresh" content="2;url=/upload_done.html">')  # Redirect after 2s
else:
    print("<h1>Error: No file uploaded</h1>")

print("</body></html>")
