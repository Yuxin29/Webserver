/*!
 * @file http_concepts.md
 * @brief team_work plan for webserver
 *
 * This document includes:
 * - Git Repo
 * - Team composition 
 * - Webserver workflow
 * - Weekly
 * - Debugging
 *
 * @details
 * This file uses Doxygen HTML-style block comments
 * Created by Yuxin for practicing documentation
 */

# Git Repo

We have a 3-person shared repo created by Lucio and Yyuxin have a local repo for backup

## Process

- Lucio started the repo and invited me, we will have 3 different branch there, I ren amed it team 
- I have a remote link repo origin 
- then at team repo, I git remote add origin https://github.com/Yuxin29/Webserver.git 
- I will work in shared team repo, and git push origin my_branch / main normally there 
- to update it to my personal redo, I do git push -u origin my_branch / main there 
- when project is over, I WILL git remote remove team


# Team composition 

This file collects several foundational concepts required when implementing an HTTP server from scratch.

## Team merbers

- Lucio **A**
- Yuxin  **B**
- Lin   **C**

## Word of each team merbers

### A |Network & I/O Manager |- Set up listening sockets		
-Handle poll() / select() / epoll()
-Manage client connections and file descriptors | - Implement non-blocking I/O logic
-Manage read/write events
-Prevent blocking to ensure responsiveness | - Optimize performance under heavy load
-Handle signals and interruptions
Conduct stress tests (e.g., with ab, wrk) |

### B | HTTP Request/Response Handler | - Implement HTTP request parser (start-line, headers, body)
-Build HTTP response generator (status line, headers, body)
-Handle basic GET requests | - Add support for POST and DELETE
-Ensure HTTP/1.1 compliance
-Implement proper status codes and headers | - Fine-tune keep-alive behavior
-Match browser and NGINX behavior
-(Optional??) Handle chunked transfer encoding |

### C | Configuration & CGI Manager | - Design and implement server.conf parser
-Support directives: server, location, listen, root, index, etc. | - Implement CGI execution (e.g., via execve)
-Add file upload support
-Route requests and handle custom error pages | - Add bonus features: autoindex, redirects
-Prepare demo config files for defense
-Polish docs, error handling, testing scripts |


# Webserver workflow

Below is the full ASCII architecture diagram of how the server handles a request.

Doxygen note:  
ASCII diagrams must be placed inside triple backticks to render properly.

```
             ┌────────────────────┐
             │     Client         │
             └─────────┬──────────┘
                       │  HTTP request
                       ▼
         ┌───────────────────────────────────┐
         │       A: Network & I/O Manager    │
         │---------------------------------- │
         │ - Accepts connections             │
         │ - poll/select/epoll monitors FDs  │
         │ - Non-blocking read/write         │
         └───────────┬───────────────────────┘
                     │ raw bytes
                     ▼
        ┌─────────────────────────────────────┐
        │   B: HTTP Request/Response Handler  │
        │-------------------------------------│
        │ - Parse HTTP request                │
        │ - Apply HTTP rules                  │
        │ - Determines action (static/CGI/etc)│
        └──────────┬───────────────┬──────────┘
                   │               │
       asks for config rules   needs CGI process
                   │               │
                   ▼               ▼
 ┌──────────────────────────────────────────────┐
 │        C: Configuration & CGI Manager        │
 │----------------------------------------------│
 │ - server.conf parser                         │
 │ - location/root/index/error_pages            │
 │ - CGI execution (execve)                     │
 │ - file path resolution                       │
 └──────────────────┬───────────────────────────┘
                    │ response data (file/CGI output)
                    ▼
        ┌─────────────────────────────────────┐
        │    B builds full HTTP response      │
        └──────────────────┬──────────────────┘
                           │ final bytes
                           ▼
         ┌────────────────────────────────────┐
         │    A: Sends response to client     │
         └────────────────────────────────────┘
```
# If I want to finish my HTTP project in 3 weeks
    Legend: ✅ = done, ❌ = missing, ⏳ = in progress, ⚠️ = caution

## Week 1 — HTTP Parser + Request Handling
**Goal:** Parse a complete HTTP request (start-line, headers, body)  

### Day 1 — Architecture Setup + HTTP Basics  ✅
Learning:
- HTTP/1.1 structure
- Request-Line, Headers, CRLF rules
- Body handling (Content-Length)
Implement:
- `HttpRequest` and `HttpParser` class skeletons

### Day 2 — `parseRequestLine()`  ✅
Learning:
- Methods: GET, POST, DELETE  
- URL format  
- HTTP version validation  
Implement:
- `parseRequestLine()`
Note: Person A will forward raw received data to you.

### Day 3 — `parseHeaders()`  ✅
Learning:
- `key: value` rules  
- Case-insensitive header names  
- Header merging  
- `Host` header required (HTTP/1.1)
Implement:
- `parseHeaders()`
Test:
- Duplicate headers  
- Missing CRLF cases

### Day 4 — `parseBody()` (Content-Length)  ✅
Learning:
- Content-Length processing  
- Body size limits (prevent 413 Payload Too Large)
Implement:
- `parseBody()`

### Day 5 — State Machine (required for non-blocking I/O)  ✅
Learning:
- Parser states: `START_LINE`, `HEADERS`, `BODY`, `DONE`
Implement:
- `HttpParser::consume(buffer)`  
  → must support partial input (slowloris defense)
Test:
- Fragmented TCP packets  
- Byte-by-byte input simulating slow attacks

## Week 2 — HTTP Response + Method Handling
**Goal:** Generate proper HTTP/1.1 responses

### Day 6 — Response Builder  ✅
Learning:
- Status line  
- Headers  
- Content-Length  
- Connection header
Implement:
- `HttpResponse::buildResponseString()`
Test:
- Compare output with NGINX:

### Day 7 — GET (Static File)  ✅
Learning:
- MIME types  
- File permissions  
- Optional: index file support
Implement:
- GET handler

### Day 8 — POST  ⏳
Learning:
- Saving request body  
- Safe path handling  
- Prevent directory traversal (`../`)
Implement:
- POST handler → write body to file

### Day 9 — DELETE  ⏳
Learning:
- Permission checks  
- Non-existent file → 404  
- Directory → 403 (match NGINX behavior)
Implement:
- DELETE handler

### Day 10 — Error Handling  ✅
Learning:
- 400 Bad Request  
- 404 Not Found  
- 413 Payload Too Large  
- 414 URI Too Long  
- 500 Internal Server Error  
Implement:
- `makeErrorResponse()`
Test:
- All invalid inputs  
- Oversized URL  
- Large body input

## Week 3 — HTTP Behavior, Browser Compatibility, Stress Tests

### Day 11 — Keep-Alive Behavior  ✅
Learning:
- `Connection: keep-alive`  
- `Connection: close`  
- HTTP/1.1 default: keep-alive  
- HTTP/1.0 default: close  
Implement:
- `shouldCloseConnection()` (decide connection persistence)
Test:
- `curl -v localhost:8080`
- Chrome keep-alive validation

### Day 12 — Matching NGINX Behavior  ✅
Learning:
- Handling extra spaces  
- Duplicate headers  
- Missing Host → 400  
- Accept: */* compatibility  
Implement:
- Parser tolerance improvements

### Day 13 — Chunked Encoding (Optional)  ✅
Implement:
- Chunked request parsing OR chunked responses

### Day 14 — Stress Testing
Test:
- No memory leaks (parser)  
- No double free  
- No keep-alive infinite loops  

### Day 15 — Final Cleanup
- Code cleanup  
- Documentation  
- Remove debug prints  
- RFC validation  
- Real browser testing  

# Debugging
  below are bugs my team found during the ddevelopment

## 1. path traversal  
  user should not be able to visit other locations other than root folders.
  example of failing test: echo -e "GET /../../lin_note HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080
### how
  used std::filesystem::canonical to get the real path and check if it is under root
  modified functions: std::string mapUriToPath(const config::LocationConfig* loc, const std::string& uri_raw)

## 2.error page
  the ready made error page should be returned from site/static/error/xxx.html
### how
  modified functions: HttpResponse makeErrorResponse(int status, const config::ServerConfig* vh)

## 3.Autoindex 
  When autoindex is off, navigating to a directory such as: http://localhost:8080/files
  should not show any directory listing.

  Expected behavior (consistent with Nginx):
  If autoindex = off and the directory has an index file → return the index file
  If autoindex = off and the directory has no index file → return 403 Forbidden

  Never generate a directory listing when autoindex is off
  Currently, the reason you are seeing a file list is because the directory contains: files/index.html
  which is a manually written file, not autoindex output.

### how
  Lin removed defaulautoindex

To be fixed:
1. content length 0
% ./tester http://localhost:8080
