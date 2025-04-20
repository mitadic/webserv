import pytest
import socket
from urllib.parse import urlparse

CODE_200 = [

	# simple GET, has at least one header as required
	"GET / HTTP/1.1\r\nConnection: keep-alive\r\n\r\n",

	# any number of whitespaces before the header value
	"GET / HTTP/1.1\r\nConnection:                keep-alive\r\n\r\n",

	# more kinds of whitespaces before the header value
	"GET / HTTP/1.1\r\nConnection:\t\t \tkeep-alive\r\n\r\n",

	# GET has body with length
	("GET / HTTP/1.1\r\n"
		"Content-Length: 3\r\n"
		"\r\n"
		"abc"),

	# GET has body too long, this needs updating for multiple requests if pipelining
	("GET / HTTP/1.1\r\n"
		"Content-Length: 3\r\n"
		"\r\n"
		"abcdefghijklmnopqrstuvwxyz"),

	# nonsense header but well formatted
	"GET / HTTP/1.1\r\nFruit: banana\r\n\r\n",

	# Wrapped header -- note that our wrapping isn't too great bc not taking into consideration the header-specific delimiters, but RFC 7230 delegalizes wrapping anyway, so
	("GET / HTTP/1.1\r\n"
		"Connection: keep-alive\r\n"
		"\tclose\r\n"
		" keep-alive\r\n"
		"\t banana\r\n"
		"\t  \t keep-alive\r\n"
		"Host: 127.0.0.1:8080\r\n\r\n"),
]

MALFORMED_REQUESTS_400 = [

	# simple GET, lacking any headers
	"GET / HTTP/1.1\r\n\r\n",

	# double slash uri
    ("POST /cgi-bin//?alias=name HTTP/1.1\r\n"
		"Content-Length: 3\r\n"
		"Content-Type: application/json\r\n"
		"\r\n"
		"{J}"),

	# extra junk
	"GET / HTTP/1.1 junk_here\r\nConnection: keep-alive\r\n\r\n",

	# missing URI
	"GET HTTP/1.1\r\nConnection: keep-alive\r\n\r\n",

	# v malformed
	"GET / HTTP/1.1.0\r\nConnection: keep-alive\r\n\r\n",

	# missing v
	"GET / \r\nConnection: keep-alive\r\n\r\n",

    # v negative
	"GET / HTTP/-1.1\r\nConnection: keep-alive\r\n\r\n",

    # v missing dot
	"GET / HTTP/10\r\nConnection: keep-alive\r\n\r\n",

    # v huge
	"GET / HTTP/2400000000.0\r\nConnection: keep-alive\r\n\r\n",

    # v empty string
	"GET / HTTP/\"\"\r\nConnection: keep-alive\r\n\r\n",

	# multiple transfer-encodings
	"POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\nTransfer-Encoding: chunked\r\n\r\n",

	# negative content-length
	"POST / HTTP/1.1\r\nContent-Length: -1\r\n\r\n",

	# incorrect content-length  -- no such thing
	# "POST / HTTP/1.1\r\nContent-Length: 4\r\n\r\nabc",

	# port in Host too large
	"GET / HTTP/1.1\r\nHost: localhost:70123\r\n\r\n",

	# port in Host don't match?
	"GET / HTTP/1.1\r\nHost: localhost:65222\r\n\r\n",

	# bad Host number1
	"GET / HTTP/1.1\r\nHost: 1277.0.0.1:8080\r\n\r\n",

	# bad Host number2
	"GET / HTTP/1.1\r\nHost: 127.0002.0.1:8080\r\n\r\n",

	# bad Host number3
	"GET / HTTP/1.1\r\nHost: 127.0.0:8080\r\n\r\n",

	# bad Host number4
	"GET / HTTP/1.1\r\nHost: 127.0.0.-22:8080\r\n\r\n",

	# bad Host number5
	"GET / HTTP/1.1\r\nHost: 123456789:8080\r\n\r\n",

	# missing value i.e. semicolon in final pos of header
	"GET / HTTP/1.1\r\nFruit:\r\nConnection: keep-alive\r\n\r\n",

	# missing key i.e. semicolon in first pos of header
	"GET / HTTP/1.1\r\n: banana\r\n\r\n",

	# LWS before header key
	"GET / HTTP/1.1\r\n Connection: keep-alive\r\n\r\n",

	# LWS before ':' delimiter
	"GET / HTTP/1.1\r\nConne ction: keep-alive\r\n\r\n",

]

UNSUPPORTED_VERSIONS_505 = [
	# v too big
	"GET / HTTP/1.2\r\nContent-Length: 3\r\n\r\n",

	# v too small
	"GET / HTTP/0.8\r\nContent-Length: 3\r\n\r\n",
]

CODE_501 = [
	# nonsense method
	("BANANA / HTTP/1.2\r\n"
		"Content-Length: 3\r\n"
		"\r\n"),
]

TIMEOUT_408 = [

	# barrage of nonsense without delimiter
	"GET / HTTP/1.1\r\nHost: localhost:70123GET / HTTP/1.1\r\nHost: localhost:65222",

	# nonsense length which never occurs
	("GET / HTTP/1.1\r\n"
		"Content-Length: 3\r\n"
		"\r\n"),
]

@pytest.mark.parametrize("request_raw", CODE_200)
def test_200(webserver, base_url, request_raw):
	host = urlparse(base_url).hostname
	port = urlparse(base_url).port
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((host, port))
		s.sendall(request_raw.encode('utf-8'))
		response = s.recv(4096).decode('utf-8')
	assert "200" in response

@pytest.mark.parametrize("request_raw", MALFORMED_REQUESTS_400)
def test_400(webserver, base_url, request_raw):
	host = urlparse(base_url).hostname
	port = urlparse(base_url).port
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((host, port))
		s.sendall(request_raw.encode('utf-8'))
		response = s.recv(4096).decode('utf-8')
	assert "400" in response

@pytest.mark.parametrize("request_raw", UNSUPPORTED_VERSIONS_505)
def test_505(webserver, base_url, request_raw):
	host = urlparse(base_url).hostname
	port = urlparse(base_url).port
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((host, port))
		s.sendall(request_raw.encode('utf-8'))
		response = s.recv(4096).decode('utf-8')
	assert "505" in response

@pytest.mark.parametrize("request_raw", CODE_501)
def test_501(webserver, base_url, request_raw):
	host = urlparse(base_url).hostname
	port = urlparse(base_url).port
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.connect((host, port))
		s.sendall(request_raw.encode('utf-8'))
		response = s.recv(4096).decode('utf-8')
	assert "501" in response

# @pytest.mark.parametrize("request_raw", TIMEOUT_408)
# def test_408(webserver, base_url, request_raw):
# 	host = urlparse(base_url).hostname
# 	port = urlparse(base_url).port
# 	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
# 		s.connect((host, port))
# 		s.sendall(request_raw.encode('utf-8'))
# 		response = s.recv(4096).decode('utf-8')
# 	assert "408" in response
