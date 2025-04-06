#!/usr/bin/env python3

import os
import sys
import cgi

UPLOAD_DIR = "www/secondary/uploads/" #os.environ.get("PATH_INFO", "")
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Read form data
form = cgi.FieldStorage()

# Get the uploaded file
file_item = form["file"]

# allowed_extensions = {".txt", ".jpg", ".png", ".pdf"}
# ext = os.path.splitext(file_item.filename)[1].lower()
# if ext not in allowed_extensions:
#     # Trigger error response
#     print("Status: 400 Bad Request")
#     print("Content-Type: text/html\r\n\r\n", end="")
#     print(f"""<html>
#         <head><title>Upload Error</title></head>
#         <body>
#             <h1>400 Bad Request</h1>
#             <p>File type '{html.escape(ext)}' is not allowed.</p>
#             <a href="/upload_form.html">Go back</a>
#         </body>
#     </html>
#     """)
#     exit()

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
