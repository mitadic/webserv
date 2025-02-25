#pragma once

#include "CgiHandler.hpp"


#define ACCEPT "Accept"
#define ACCEPTCHARSET "Accept-Charset"
#define ACCEPTENCODING "Accept-Encoding"
#define ACCEPTLANGUAGE "Accept-Language"
#define ACCEPTRANGES "Accept-Ranges"
#define AGE "Age"
#define ALLOW "Allow"
#define AUTHORIZATION "Authorization"
#define CACHECONTROL "Cache-Control"
#define CONNECTION "Connection"
#define CONTENTENCODING "Content-Encoding"
#define CONTENTLANGUAGE "Content-Language"
#define CONTENTLENGTH "Content-Length"
#define CONTENTLOCATION "Content-Location"
#define CONTENTMD5 "Content-MD5"
#define CONTENTRANGE "Content-Range"
#define CONTENTTYPE "Content-Type"
#define DATE "Date"
#define ETAG "ETag"
#define EXPECT "Expect"
#define EXPIRES "Expires"
#define FROM "From"
#define HOST "Host"
#define IFMATCH "If-Match"
#define IFMODIFIEDSINCE "If-Modified-Since"
#define IFNONEMATCH "If-None-Match"
#define IFRANGE "If-Range"
#define IFUNMODIFIEDSINCE "If-Unmodified-Since"
#define LASTMODIFIED "Last-Modified"
#define LOCATION "Location"
#define MAXFORWARDS "Max-Forwards"
#define PRAGMA "Pragma"
#define PROXYAUTHENTICATE "Proxy-Authenticate"
#define PROXYAUTHORIZATION "Proxy-Authorization"
#define RANGE "Range"
#define REFERER "Referer"
#define RETRYAFTER "Retry-After"
#define SERVER "Server"
#define TE "TE"
#define TRAILER "Trailer"
#define TRANSFERENCODING "Transfer-Encoding"
#define UPGRADE "Upgrade"
#define USERAGENT "User-Agent"
#define VARY "Vary"
#define VIA "Via"
#define WARNING "Warning"
#define WWWAUTHENTICATE "WWW-Authenticate"


class Request {
public:
    Request();
    ~Request();
	Request(const Request&);

	void reset();
	void reset_client();

	void parse();

	std::string host;            // Host: example.com
	std::string mime_type;       // Content-Type: application/json (refers to own payload)
	std::string request;
    std::string response;
	int			total_sent;
	long long   content_length;  // Content-Length: 27
	short       method;          // GET POST DELETE
	int         client_fd;
	bool		timed_out;
	bool		await_reconnection;

	int			cgi_status;
	CgiHandler  cgi;
	std::string cgi_job_id;
	std::string cgi_output;

private:

};