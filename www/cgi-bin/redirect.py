#!/usr/bin/env python3

#bad loc
#print("Location: ./hello_banana_world.py", end="\r\n\r\n")

# this is a CGI script with 302 redirection
print("Status: 302 Found", end="\r\n")
print("Location: ./hello_world.py", end="\r\n\r\n")
