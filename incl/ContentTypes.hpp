#pragma once

#define CONTENT_TYPES_N 12

enum e_content_types {
    TEXT_PLAIN = 0,
    TEXT_HTML,
    TEXT_XML,
    APPLICATION_XML,
    APPLICATION_XHTML_XML,
    APPLICATION_OCTET_STREAM,
    APPLICATION_X_WWW_FORM_URLENCODED,  // form submissions
    MULTIPART_FORM_DATA,  // file uploads
    IMAGE_GIF,
    IMAGE_JPEG,
    IMAGE_PNG,
	MEDIA_MP3
};

extern const char *content_types[CONTENT_TYPES_N];