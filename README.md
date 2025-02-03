# webserv

```C++
#include <cctype>    // std::tolower
#include <algorithm> // std::equal

bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool iequals(const std::string& a, const std::string& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), ichar_equals);
}
```

## Resources bundle

### Web

What | Comment
:--- | :---
All [section (2) man pages](https://man7.org/linux/man-pages/dir_section_2.html) | -
webserv MVP [4min article](https://medium.com/@ahmadesekak/setting-up-a-server-using-c-sockets-124e404593c9) | by `asekkak`
webserv MVP [5min article](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7) | same proj team, different person
Nginx [DirectoryStructure](https://wiki.debian.org/Nginx/DirectoryStructure) | -
Nginx [configuration](https://www.solo.io/topics/nginx/nginx-configuration) | basic components && what to avoid
[rfc2616](https://datatracker.ietf.org/doc/html/rfc2616) | the OG HTTP/1.1 rfc

### Abbrev reminders

- NIC : Network Interface Card
- TCP : Transmission Control Protocol (socket() int type `SOCK_STREAM`)
- UDP : User Datagram Protocol (socket() int type `SOCK_DGRAM`)
- TCP/IP : The internet protocol suite, a framework for organizing comm. protocols
- IPC : Inter-Process Communication (linux paradigm, multisystem compatible)
- DHCP : Dynamic Host Configuration Protocol

### Terminology with web resource links
- [Server blocks](https://www.digitalocean.com/community/tutorials/how-to-set-up-nginx-server-blocks-virtual-hosts-on-ubuntu-16-04) (nginx) == Virtual hosts (apache) ==> can be used to encapsulate configuration details and host more than one domain on a single server

### Linux recap notes

#### System-wide FD tracking

```bash
cat /proc/sys/fs/file-nr
#allocated #allocated-but-unused(i.e."free") #max
```

#### Non-blocking I/O -- FD readiness verification

##### Atomicity (Kerrisk, page 90):
> Atomicity is a concept [where] the kernel guarantees that all of the steps in [an atomic] operation are completed without being interrupted by another process or thread.

##### Blocking (Kerrisk, page 1325):

[YT vid showcasing blocking sockets](https://youtu.be/Y5PiHboUctw?feature=shared&t=125)

> Most of the [core linux programs] employ an I/O model under which a process performs I/O on just one file descriptor at a time, and each I/O system call blocks [the thread] until the data is transferred.

##### Multiplexing (Kerrisk, page 1327):
> In effect, I/O multiplexing, signal-driven I/O, and epoll are all methods of achieving the same result—monitoring one or, commonly, several file descriptors simultaneously to see if they are ready to perform I/O (to be precise, to see whether an I/O system call could be performed without blocking). The transition of a file descriptor into a ready state is triggered by some type of I/O event, such as the arrival of input, the completion of a socket connection, or the availability of space in a previously full socket send buffer after TCP transmits queued data to the socket peer.

> Monitoring multiple file descriptors is useful in applications such as network servers that must simultaneously monitor multiple client sockets, or applications that must simultaneously monitor input from a terminal and a pipe or socket. Note that none of these techniques performs I/O. They merely tell us that a
file descriptor is ready. Some other system call must then be used to actually perform the I/O.

##### I/O readiness (Kerrisk, page 1329):
> - Level-triggered notification (select(), poll()): A file descriptor is considered to be ready if it is possible to perform an I/O system call without blocking.
> - Edge-triggered notification (signal-driven I/O): Notification is provided if there is I/O activity (e.g., new input) on a file descriptor since it was last monitored.

`epoll()` is capable of both checks.

### Shenya takeaways

1. socket_fd is only used for TCP header I/O, whereas the copies of it which accept spawns are what works with the TCP data (the actual request)
2. No parallelism - that's why there's a need for manual load balancing via "chunking"
3. Handling only server blocks in the config is sufficient
4. accept() == epoll_wait() == make the passive socket now...active?
5. `ulimit -n` will be the limit of connections. Use precautionary `while ((test_fd = open("/dev/null", O_RDONLY)) != -1)` or sth

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
