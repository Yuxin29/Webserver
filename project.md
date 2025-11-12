# Web Server Project

## Overview

Webserver project with a WebServer manager that parses the config and creates Server instances. Each Server represents one server block and handles its own connections. 
When data arrives, the Server delegates to Http for protocol handling. Http parses the request, routes it (static file or CGI), and generates the response. 
The CGI class handles script execution separately. Everything runs in a single non-blocking event loop using poll().

---

## Architecture Components

### 1. WebServer (Manager/Orchestrator)
- **Purpose**: Application entry point and lifecycle manager
- **Responsibilities**:
  - Parse configuration file
  - Create and manage multiple `Server` instances (virtual hosts)
  - Use `poll()`/`epoll()` to monitor all server and client sockets
  - Handle signals for graceful shutdown (SIGINT, SIGTERM)
  - Coordinate the main event loop
- **Location**: `main.cpp` or dedicated `WebServer` class

### 2. Server (Virtual Host/Server Instance)
- **Purpose**: Represents ONE server block from the config file
- **Responsibilities**:
  - Own ONE listening socket (specific port + host binding)
  - Store configuration for this server block (server_name, root, locations, etc.)
  - Accept new client connections
  - Manage connected client file descriptors
  - Read raw data from client sockets
  - Delegate HTTP processing to the `Http` class
  - Send formatted responses back to clients
  - Close connections when complete
- **Key Point**: Each `Server` instance = one server block in config

### 3. Http (HTTP Protocol Handler)
- **Purpose**: HTTP protocol processor and request router
- **Responsibilities**:
  - Parse raw HTTP request data → create `Request` objects
  - Validate HTTP syntax and version
  - Route requests based on path and method:
    - Static file serving
    - CGI script execution
    - Error responses (404, 405, 500, etc.)
  - Generate complete HTTP responses (status line + headers + body)
  - Call `CGI` handler when request matches CGI path
  - Handle different HTTP methods (GET, POST, DELETE)
  - Manage HTTP headers (Content-Type, Content-Length, etc.)
- **Key Point**: This is the "brain" that understands HTTP protocol

### 4. Request (Data Container)
- **Purpose**: Stores parsed HTTP request data
- **Responsibilities**:
  - HTTP method (GET, POST, DELETE, etc.)
  - Request URI and path
  - HTTP version
  - Headers (stored as key-value map)
  - Request body (for POST/PUT)
  - Query parameters (parsed from URI)
  - Client information (socket fd, IP address)
- **Key Point**: Pure data structure with getters/setters

### 5. CGI (External Script Executor)
- **Purpose**: Execute CGI scripts and capture their output
- **Responsibilities**:
  - Set CGI environment variables (REQUEST_METHOD, QUERY_STRING, CONTENT_TYPE, etc.)
  - Fork child process
  - Execute the script (Python, PHP, Perl, etc.)
  - Pipe request body to script's stdin
  - Capture script's stdout (headers + body)
  - Parse CGI output to separate headers from body
  - Handle script timeouts and errors
  - Return formatted output to `Http` class
- **Key Point**: Only called when needed for CGI requests

---

## Request Handling Workflow

### Complete Flow:

```
1. Client connects to Server (e.g., localhost:8080)
   └─> Server accepts connection (new client fd)

2. Client sends HTTP request
   └─> Server reads raw data from socket

3. Server passes raw data to Http::parse()
   └─> Http parses and creates Request object

4. Http analyzes the Request:
   ├─ Static file? → Http::serveStaticFile()
   ├─ CGI script?  → Http calls CGI::execute()
   └─ Invalid?     → Http::generateErrorResponse()

5. For CGI requests:
   ├─> CGI::execute(Request)
   ├─> Sets environment variables
   ├─> Forks and executes script
   ├─> Captures script output
   └─> Returns output to Http

6. Http generates complete HTTP response:
   ├─> Status line (HTTP/1.1 200 OK)
   ├─> Headers (Content-Type, Content-Length, etc.)
   └─> Body (file content or CGI output)

7. Http returns response to Server

8. Server sends response to client socket

9. Server closes connection (or keeps alive if requested)
```

### Detailed Example:

**Request**: `GET /cgi-bin/hello.py?name=Alice HTTP/1.1`

1. **WebServer** monitors all sockets with `poll()`
2. **Server:8080** detects incoming connection
3. **Server** accepts and reads: `"GET /cgi-bin/hello.py?name=Alice HTTP/1.1\r\nHost: localhost\r\n..."`
4. **Server** calls: `Http::parseRequest(rawData)`
5. **Http** creates:
   ```
   Request {
     method: "GET"
     path: "/cgi-bin/hello.py"
     query: "name=Alice"
     headers: {"Host": "localhost", ...}
   }
   ```
6. **Http** checks path → matches CGI location
7. **Http** calls: `CGI::execute(request)`
8. **CGI**:
   - Sets `REQUEST_METHOD=GET`, `QUERY_STRING=name=Alice`
   - Forks and executes `/path/to/cgi-bin/hello.py`
   - Captures output: `"Content-Type: text/html\r\n\r\n<html>Hello Alice!</html>"`
9. **Http** wraps in HTTP response:
   ```
   HTTP/1.1 200 OK
   Content-Type: text/html
   Content-Length: 28
   
   <html>Hello Alice!</html>
   ```
10. **Server** sends to client
11. **Server** closes connection

---

## Class Relationships

```
┌─────────────────┐
│   WebServer     │ ← Entry point (main.cpp or WebServer class)
│   (Manager)     │
└────────┬────────┘
         │ creates & manages
         │
         ├──────────┬──────────┬──────────┐
         ▼          ▼          ▼          ▼
   ┌─────────┐┌─────────┐┌─────────┐┌─────────┐
   │Server   ││Server   ││Server   ││Server   │
   │ :8080   ││ :8081   ││ :8082   ││ :9000   │
   └────┬────┘└────┬────┘└────┬────┘└────┬────┘
        │          │          │          │
        │ each Server has one Http instance
        ▼          ▼          ▼          ▼
   ┌─────────┐┌─────────┐┌─────────┐┌─────────┐
   │  Http   ││  Http   ││  Http   ││  Http   │
   └────┬────┘└────┬────┘└────┬────┘└────┬────┘
        │          │          │          │
        │ creates Request objects per client request
        ▼
   ┌──────────┐
   │ Request  │ ← Data container (created per request)
   └──────────┘
        │
        │ passed to CGI when needed
        ▼
   ┌──────────┐
   │   CGI    │ ← Called on-demand for script execution
   └──────────┘
```

---

## Key Design Principles

### Why Http Creates Request (Not Server)?

**The Question**: Why does `Http` create the `Request` object? Why not create it in `Server`?

**The Answer**: **Separation of concerns** - each class should have ONE responsibility.

**Server's Responsibility (Network Layer)**:
- Handle sockets and raw bytes
- `recv()` to read data, `send()` to write data
- Doesn't understand what the bytes mean
- Just knows: "Data arrived on socket, pass it along"

**Http's Responsibility (Protocol Layer)**:
- Understand HTTP protocol syntax
- Parse raw bytes into structured data (Request object)
- Know about methods, headers, status codes
- Doesn't touch sockets

**How Server Reads Request Data**:
Server doesn't need to! Here's the flow:
1. Server reads raw bytes from socket
2. Server passes bytes to Http: `http.processRequest(rawData)`
3. Http creates Request internally and processes it
4. Http returns formatted response string
5. Server sends response bytes to socket

**Think of it like a restaurant**:
- **Server** = Waiter (takes order slip, delivers food - doesn't read the order)
- **Http** = Chef (reads order slip, creates recipe card = Request object)

Server never needs to access Request directly - it just moves bytes in and out.

### Separation of Concerns:
- **WebServer**: Orchestration and lifecycle
- **Server**: Network I/O and connection management
- **Http**: HTTP protocol logic
- **Request**: Data storage only
- **CGI**: External process execution

### Data Flow:
```
Raw socket data → Server → Http → Request → CGI (if needed) → Http → Server → Client
```

### Scalability:
- Multiple server blocks (virtual hosts) on different ports
- Each Server handles its own clients
- Single event loop monitors all sockets efficiently

### Extensibility:
- Easy to add new HTTP methods in `Http` class
- CGI can be extended to FastCGI
- Server can support different protocols
- Configuration-driven behavior

---


