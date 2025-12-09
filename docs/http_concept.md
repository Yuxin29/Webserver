/*!
 * @file http_concepts.md
 * @brief Core C++ and HTTP concepts used in building an HTTP server.
 *
 * This document includes:
 * - std::map usage
 * - Finite State Machines for parsing
 * - std::istringstream
 * - HTTP rules for GET/POST/DELETE
 * - Buffer handling (section will expand)
 * - MIME types (section will expand)
 * - Sockets (section will expand)
 * - Nginx overview
 *
 * @details
 * This file uses Doxygen HTML-style block comments
 * Created by Yuxin for practicing documentation
 */

# HTTP Server Concepts

This file collects several foundational concepts required when implementing an HTTP server from scratch.

---

# 1. `std::map` — Ordered Key–Value Storage

`std::map` is widely used to store HTTP headers.

## Properties

- Keys are **unique**
- Keys remain **sorted**
- Operations (lookup/insertion/deletion) are **O(log n)**
- Implemented using a **Red–Black tree**

## Example

```cpp
#include <map>
#include <string>

std::map<std::string, std::string> headers;

headers["Host"] = "localhost:8080";
headers["User-Agent"] = "curl/7.68.0";
headers["Accept"] = "*/*";
headers["Content-Length"] = "12";



====================================================== Nginx ======================================================
Nginx: Engine-X
it is a high-performance web server, reverse proxy and load balancer designed  to handle massive ammont of connnxtions using event-drive ynchronous architecture.
It is used for:
   - serving static files
   - Load balancing across multiple backend servers
   - ...

1. <start-line>\r\n
Method sp _path sp version crlf(Carriage Return and Line Feed)
Get /index.html HTTP/1.1\r\n
------
2. <header1: value>\r\n
Host: example.com\r\n                ← Headers
User-Agent: curl/7.64.1\r\n
Content-Length: 5\r\n
------
3. empty line
\r\n 
------
4. Body, only for POST, PUT
hello 

----> a normal http response form
<status-line>\r\n
<header1: value>\r\n
...\r\n
\r\n
<body>
------
1.<status-line>\r\n
VERSION SP STATUS_CODE SP REASON_PHRASE CRLF
HTTP/1.1 200 OK\r\needs
2.<headern: value>\r\n
Content-Type: text/html\r\n
Content-Length: 13\r\n
3. empty line
\r\need
4. body:
a string: like "hello world"

====================================================== State Machine ======================================================
State Machine / Finite State Machine(FSM). it has 3 elements.
1. State,
2. Event / Input   
3. Transition
check state -> get input -> do a thin -> change to next state
---> why do we use state machine in http request parsing
- raw Line comes with a certain order 
- it can avoid lots of if / else if, the step is tracable
- the might be interruption / waiting time between input


====================================================== http rules for GET POST DELETE ======================================================
| Method     | Can have a body?  | "Content-Length" needed?           | "Content-Type" needed?   | Notes                                                                                                                             |
| ---------- | ----------------- | ---------------------------------- | ------------------------ | --------------------------------------------------------------------------------------------------------------------------------- |
| **GET**    | Rare / Usually no | Not needed                         | Optional                 | GET requests normally do **not have a body**. Some servers allow it, but it’s unusual and most clients/servers ignore it.         |
| **POST**   | Yes               | **Required**                       | Optional but recommended | POST is the standard method to send a body (e.g., form data, JSON). `Content-Length` is necessary to know how many bytes to read. |
| **DELETE** | Rarely            | Required if body exists            | Optional                 | HTTP spec allows a body, but it’s unusual. If your parser sees a body, `Content-Length` is needed to parse it.                    |


1. HTTP/1.1 Keep-Alive

By default, HTTP/1.1 connections are persistent (keep-alive), unless the server sends Connection: close in its response. 
This means the TCP connection can be reused for multiple requests.

====================================================== A MIME type  ======================================================
A MIME type: Multipurpose Internet Mail Extension type:
-> is a std string that tells a browser or client what kind of file is being transmitted
-> format: type / subtype
   eg: text/html, image/png, application/json, text/plain, application/octec-stream
-> How it is used in a server:
   Content-Type: text/html

====================================================== Lin ConfigParser / Config    ======================================================
Lin ConfigParser / Config class
- root 
- location 
- index 
- error_page 
- cgi_path（如果是 CGI）: could be like /var/www/cgi-bin/my_script.cgi or  /usr/bin/php-cgi
- upload_path（POST/PUT） 

====================================================== CGI path ======================================================
CGI path is the filesystem path to the cgi executable program:
CGI: common gateway Interface

====================================================== Nginx ======================================================
Nginx: Engine-X
it is a high-performance web server, reverse proxy and load balancer designed  to handle massive ammont of connnxtions using event-drive ynchronous architecture.
It is used for:
   - serving static files
   - Load balancing across multiple backend servers
   - ...

====================================================== Edge case for Http request ======================================================
GET  HTTP/1.1
Host: localhost:8080
User-Agent: curl/8.3.0
Accept: */*

here the request path is empty:
- Nginx and Apache tolerate this: they treat an empty request-target as / (the root).
- Some stricter HTTP parsers will return 400 Bad Request because the request-target is required by the spec (RFC 9110).  ⚠️ YUXIN WANTS TO FOLLOW THIS


// how to check the content of a full HTTP request and respongs
// eg: curl -v http://localhost:8080/test.txt
/*
> GET /test.txt HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.68.0
> Accept: *
< HTTP/1.1 200 OK
< Content-Length: 12
< Content-Type: text/plain
< Connection: keep-alive
< Hello World
*/

// > Request  
// < Reponse 
Stopping servers listening on port: 8081

====================================================== Nginx ======================================================
Nginx: Engine-X
it is a high-performance web server, reverse proxy and load balancer designed  to handle massive ammont of connnxtions using event-drive ynchronous architecture.
It is used for:
   - serving static files
   - Load balancing across multiple backend servers
   - ...
