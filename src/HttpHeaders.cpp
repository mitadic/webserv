#include "../incl/HttpHeaders.hpp"

/*

const char *http_header_names[HTTP_HEADERS_N] =
{
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Accept-Ranges",
	"Age",					// age-value WHERE age-value = delta-seconds
	"Allow",				// #Method 
	"Authorization",
	"Cache-Control",
	"Connection",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Date",
	"ETag",
	"Expect",
	"Expires",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Last-Modified",
	"Location",
	"Max-Forwards",
	"Pragma",
	"Proxy-Authenticate",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"Retry-After",
	"Server",
	"TE",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"User-Agent",
	"Vary",
	"Via",
	"Warning",
	"WWW-Authenticate"
};

const char *http_repeatable_headers[26] =
{
	// general
	"Cache-Control",
	"Connection",
	"Pragma",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"Via",
	"Warning",

	// request
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Expect",
	"If-Match",
	"If-None-Match",
	"Range",
	"TE",
	"User-Agent", // rarely
	
	// response
	"Accept-Ranges",
	"Proxy-Authenticate",
	"Server",  // rarely
	"Vary",
	"WWW-Authenticate",

	// entity
	"Allow",
	"Content-Encoding",
	"Content-Language"
};

const char *http_general_headers[HTTP_GENERAL_HEADERS_N] =
{
	"Cache-Control",
	"Connection",
	"Date",
	"Pragma",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"Via",
	"Warning"
};

const char *http_request_headers[HTTP_REQUEST_HEADERS_N] =
{
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Authorization",
	"Expect",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Max-Forwards",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"TE",
	"User-Agent"
};

const char *http_response_headers[HTTP_RESPONSE_HEADERS_N] =
{
	"Accept-Ranges",		// 1#( range-unit | "none" )
	"Age",
	"ETag",
	"Location",
	"Proxy-Authenticate",
	"Retry-After",
	"Server",
	"Vary",
	"WWW-Authenticate"
};

const char *http_entity_headers[HTTP_ENTITY_HEADERS_N] =
{
	"Allow",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Expires",
	"Last-Modified"
};

*/


const char *http_request_legal_headers[HTTP_REQUEST_LEGAL_HEADERS_N] =
{
	// general
	"Cache-Control",		// 1#( cache-directive ) WHERE cache-directive = cache-request-directive | cache-response-directive
	"Connection",			// 1#( token )
	"Date",					// HTTP-date
	"Pragma",				// 1#( pragma-directive )
	"Trailer",				// 1#( field-name )
	"Transfer-Encoding",	// 1#( transfer-coding )
	"Upgrade",				// 1#( product ) EX Upgrade: HTTP/2.0, SHTTP/1.3, IRC/6.9, RTA/x11
	"Via",					// 1#( [ protocol-name "/" ] protocol-version SP received-by [ SP comment ] )
	"Warning",				// 1#( warn-code SP warn-agent SP warn-text [ SP warn-date ] )

	// request
	"Accept",				// #( media-range [ accept-params ] )
	"Accept-Charset",		// 1#( ( charset | "*" )[ ";" "q" "=" qvalue ] )
	"Accept-Encoding",		// 1#( content-coding [ ";" "q" "=" qvalue ] )
	"Accept-Language",		// 1#( language-range [ ";" "q" "=" qvalue ] )
	"Authorization",		// credentials (authentication scheme and token, e.g., “Basic …” or “Digest …”)
	"Expect",				// 1#( expectation ) (commonly “100-continue”)
	"From",					// mailbox
	"Host",					// host [ ":" port ]
	"If-Match",				// 1#( entity-tag | "*" )
	"If-Modified-Since",	// HTTP-date
	"If-None-Match",		// 1#( entity-tag | "*" )
	"If-Range",				// entity-tag or HTTP-date
	"If-Unmodified-Since",	// HTTP-date
	"Max-Forwards",			// 1*DIGIT
	"Proxy-Authorization",	// credentials (similar to Authorization)
	"Range",				// range-unit "=" range-set (e.g., “bytes=500-999”)
	"Referer",				// absoluteURI
	"TE",					// 1#(transfer-coding [ ";" "q" "=" qvalue ]) (may also include extension parameters)
	"User-Agent",			// 1*( product | comment )

	// entity
	"Allow",				// 1#( method )
	"Content-Encoding",		// 1#( content-coding )
	"Content-Language",		// 1#( language-tag )
	"Content-Length",		// 1*DIGIT
	"Content-Location",		// absoluteURI
	"Content-MD5",			// 1*BASE64DIGIT (the base64-encoded MD5 sum)
	"Content-Range",		// range-unit SP content-range-spec EX “bytes 200-1000/67589”
	"Content-Type",			// media-type *(type "/" subtype ( ";" parameter ) ) EX "text/html; charset=ISO-8859-4" OR "text=html"
	"Expires",				// HTTP-date
	"Last-Modified"			// HTTP-date
};
