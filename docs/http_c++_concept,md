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



====================================================== MAP container that stores key–value pairs ======================================================
#include <map>
--> std::map
- ordered assoiative container
- automateically keeps the keys sorted
- each key must be unique
- Lookup, insertion and deletion are all O(log n)
For example
std::map<std::string, std::string> headers; 
headers["Host"] = "localhost:8080";
headers["User-Agent"] = "curl/7.68.0";
headers["Accept"] = "*/*";
headers["Content-Length"] = "12";


====================================================== std::istringstream ======================================================
std::isstringstream is a class that string flows as a stream (eg. reading from a file or socket)
(maybe a bit like a automatic getnext line)
-->>Example use
std::istringstream ss("hello world 42");
std::string a, b;
int num;
ss >> a >> b >> num;
// a = "hello"
// b = "world"
// num = 42

====================================================== iterator ======================================================
std::map<std::string, std::string>::iterator it;
iterator iterates,
it points to the any pair in the container map
it->firs    //key
it->second  //value
