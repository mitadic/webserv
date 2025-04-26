#!/usr/bin/env python3

import os
import urllib.parse

print("Content-Type: text/html", end="\r\n")
print("\r\n", end="")  # Body separator

# Parse query string manually
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string)

# Debugging: Print raw and parsed query string
#print(f"<p><strong>Raw Query String:</strong> {query_string}</p>")
#print(f"<p><strong>Parsed Query Parameters:</strong> {params}</p>")

first_name = params.get("first_name", ["barren"])[0]  # Default to "barren" if no name is given
last_name = params.get("last_name", ["world"])[0]  # Default to "world" if no name is given

# HTML response
print(
f"""<!DOCTYPE html>
<html>
<head><title>CGI Script</title></head>
<body>
    <h1>Hello, {first_name} {last_name}!</h1>
</body>
</html>
""", end="")
