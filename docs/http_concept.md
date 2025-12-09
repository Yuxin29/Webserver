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

# 1. Nginx: Engine-X
  it is a high-performance web server, reverse proxy and load balancer designed  to handle massive ammont of connnxtions using event-drive ynchronous architecture.

## Usages
    - serving static files
    - Load balancing across multiple backend servers


# 2. Http request
    xxx

## Components
    - <start-line>\r\n: Method _path version 
    - <header1: value>\r\n
    - empty line
    - Body, only for POST, PUT

## Examples
    - Get /index.html HTTP/1.1\r\n
    - Host: example.com\r\n             
    - User-Agent: curl/7.64.1\r\n
    - Content-Length: 5\r\n
    - \r\n
    - hello\r\n

# 3. Http request
    xxx

## Components
    - <status-line>\r\n: VERSION SP STATUS_CODE SP REASON_PHRASE CRLF
    - <header1: value>\r\n
    - ...\r\n
    - empty line
    - a string: like "hello world"

## Examples
    - HTTP/1.1 200 OK\r\n
    - Content-Type: text/html\r\n
    - Content-Length: 13\r\n
    - \r\n
    - hello world\r\n

# 3.  State Machine 
   Finite State Machine(FSM).

## Components
    - State,
    - Event / Input   
    - Transition

## Usages
    (why do we use state machine in http request parsing)
    check state -> get input -> do a thin -> change to next stat

##  why do we use state machine in http request parsing

- raw Line comes with a certain order 
- it can avoid lots of if / else if, the step is tracable
- the might be interruption / waiting time between input

# 4. HTTP Rules for GET, POST, and DELETE

This table summarizes how different HTTP methods handle request bodies
and which headers are required for proper parsing.

| Method     | Can have a body?      | "Content-Length" needed?       | "Content-Type" needed?       | Notes                                                                 |
|----------- |--------------------- |------------------------------- |------------------------------|---------------------------------------------------------------------- |
| **GET**    | Rare / Usually no     | Not needed                     | Optional                     | GET requests normally do **not have a body**. Some servers allow it, but it is unusual and often ignored by clients/servers. |
| **POST**   | Yes                   | **Required**                   | Optional (recommended)       | POST is the standard method for sending a body (form data, JSON, etc.). `Content-Length` is required to know the body size. |
| **DELETE** | Rarely                | Required if a body exists      | Optional                     | HTTP spec allows a body in DELETE requests, but it is uncommon. If a body exists, `Content-Length` must be provided. |


# 5. A MIME type: Multipurpose Internet Mail Extension type:
    is a std string that tells a browser or client what kind of file is being transmitted

## format
    type / subtype

## Examples
    - text/html
    - image/png
    - application/json
    - text/plain
    - application/octec-stream

##  Usage
    Content-Type: text/html