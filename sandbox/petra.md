Webserv is about writing your own HTTP server in C++ 98. Throughout this project you learn about the fundamentals of communication on the World Wide Web.

<details>
<summary> Introduction </summary>

## Web Server

The term can refer to hardware or software or both working together. A web server software controls how users can access hosted files. An HTTP server can be accessed through the domain names of the websites it stores. On the hardware side, the web server stores the software and the website’s component files (HTML docs, images, CSS sheets). It connects to the internet and supports data exchange with *clients*.

### NGINX

NGINX (used by Netflix, Instagram etc.) is a high-performance web server designed for handling a large number of connections. Among many other protocols it supports HTTP/1.1 and the subject takes it as reference. Nginx has a lot of useful features that our program does not have, e.g. it is a reverse proxy, it supports SSL/TLS and streaming.

## HTTP

The Hypertext Transfer Protocol (HTTP) is an application-level
protocol for distributed, collaborative, hypermedia information
systems. We used HTTP/1.1 as the basis for our web server: for the structure of responses, requests, status codes, chunked transfers, headers and MIME types. The Request for Comments (RFC) versions 2616 (from 1999), 9112 (latest, 2022 version) and 9110 describe the protocol in detail.

Overall operation as defined in RFC 2616:

> The HTTP protocol is a request/response protocol. A client sends a
request to the server in the form of a request method, URI, and
protocol version, followed by a MIME-like message containing request
modifiers, client information, and possible body content over a
connection with a server. The server responds with a status line,
including the message's protocol version and a success or error code,
followed by a MIME-like message containing server information, entity
metainformation, and possible entity-body content.
>

</details>

<details>
<summary> The Project </summary>

## Config File

### Structure
The nginx default config file was our inspiration. We used a simple config file consisting of *server block directives* that listen on different ports. They can contain *location directives* that hold certain rules or configurations (*simple directives*).

```conf
# This is a block directive.
# A block directive consists of a name and a group of block directives or simple directives.
# A simple directive consists of a name and a value.

server {
	host 127.0.0.1;
	listen 8080;

	client_max_body_size 10485760;

	location / {
		root /var/www/html;
		index index.html;
		allowed_methods GET;
	}
}
```
<details>

<details>
<summary> Tools for Testing </summary>

### Telnet

Telnet is a network protocol (like SSH) that enables remote access to a server over the internet via a terminal. It uses TCP and was developed in the 1960. Today it is replaced mostly by other secure protocols, but can be used for troubleshooting and network testing.

```bash
telnet localhost 8080 #change the port

#send a request
GET / HTTP/1.1
Host: localhost

#expected response
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234

<html>...</html>
```

### Curl

cURL stands for client URL and it is a command line tool that developers use to transfer data to and from a server.

```bash

#-v (verbose) prints request & response headers.
#testing with a file
curl -v http://localhost:8080/index.html

#DELETE
curl -X DELETE http://localhost:8080/file.txt
 #expected response
 HTTP/1.1 204 No Content

#uploading files
curl -v -X POST http://localhost:8080/upload -F "file=@test.txt"
```

### Postman

### Developer Tools

<details>

## Our Top Resources



