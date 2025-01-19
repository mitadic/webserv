# webserv

## Resources bundle

### Links

- All [section (2) man pages](https://man7.org/linux/man-pages/dir_section_2.html)
- webserv MVP [4min article](https://medium.com/@ahmadesekak/setting-up-a-server-using-c-sockets-124e404593c9) by `asekkak`

### Abbrev reminders

- NIC : Network Interface Card
- TCP : Transmission Control Protocol (socket() int type `SOCK_STREAM`)
- UDP : User Datagram Protocol (socket() int type `SOCK_DGRAM`)
- TCP/IP : The internet protocol suite, a framework for organizing comm. protocols
- IPC : Inter-Process Communication (linux paradigm, multisystem compatible)
- DHCP : Dynamic Host Configuration Protocol

## RFC highlights

The most overarching and relevant ones would be the **HTTP/1.1** RFC [7230](https://datatracker.ietf.org/doc/html/rfc7230) through RFC 7235 (which obsolete the OG RFC 2616).

### RFC 7230 Message Syntax and Routing

Page 7

```
The following example illustrates a typical message exchange for a
GET request (Section 4.3.1 of [RFC7231]) on the URI
"http://www.example.com/hello.txt":

Client request:

    GET /hello.txt HTTP/1.1
    User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3
    Host: www.example.com
    Accept-Language: en, mi


Server response:

    HTTP/1.1 200 OK
    Date: Mon, 27 Jul 2009 12:28:53 GMT
    Server: Apache
    Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
    ETag: "34aa387-d-1568eb00"
    Accept-Ranges: bytes
    Content-Length: 51
    Vary: Accept-Encoding
    Content-Type: text/plain

    Hello World! My payload includes a trailing CRLF.
```
