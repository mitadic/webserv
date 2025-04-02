#!/usr/bin/env python3

import os
import urllib.parse

print("Content-Type: text/html")
print()

# Parse query string manually
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)

# Debugging: Print raw and parsed query string
#print(f"<p><strong>Raw Query String:</strong> {query_string}</p>")
#print(f"<p><strong>Parsed Query Parameters:</strong> {params}</p>")

name = params.get("name", ["World"])[0]  # Default to "World" if no name is given

# HTML response
print(f"""
<!DOCTYPE html>
<html>
<head><title>CGI Script</title></head>
<body>
    <h1>Hello, {name}!</h1>
</body>
</html>
""")
