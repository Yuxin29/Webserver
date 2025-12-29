# Webserver

A lightweight HTTP/1.1 web server written in C++ as part of the 42 school **webserver** project.  
This project implements a subset of Nginx-like behavior, including static file serving, CGI execution, configuration parsing, and proper HTTP error handling.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
  - [Request Flow](#request-flow)
  - [Configuration System](#configuration-system)
  - [HTTP Handling](#http-handling)
  - [CGI Execution](#cgi-execution)
- [Configuration File](#configuration-file)
- [Error Handling](#error-handling)
- [Build & Run](#build--run)
- [Testing](#testing)
- [Authors](#Authors)

---

## Overview

This webserver is designed to:

- Parse and validate HTTP/1.1 requests
- Serve static files from configured roots
- Execute CGI scripts (e.g. Python, PHP, .sh)
- Handle multiple clients using non-blocking I/O
- Load and validate Nginx-style configuration files

The implementation focuses on **clarity, correctness, and strict separation of responsibilities** between modules.

---

## Features

- HTTP methods: **GET, POST, DELETE**
- Static file serving
- Directory index support
- Custom error pages
- CGI execution via `fork()` + `execve()`
- Request body handling
- Chunked and non-chunked requests
- Multiple server blocks
- Location-based configuration
- Precise configuration error reporting (line/column)

---

## Project Structure

```
├── include # Header files
├── src # Source files
│ ├── CGI.cpp
│ ├── ConfigTokenizer.cpp
│ ├── ConfigParser.cpp
│ ├── ConfigBuilder.cpp
│ ├── HttpRequestParser.cpp
│ ├── HttpResponse.cpp
│ ├── HttpResponseHandler.cpp
│ ├── Server.cpp
│ └── Webserver.cpp
├── sites # Static files and CGI scripts
├── tester # Test utilities
└── README.md
```

---

## Architecture

### Request Flow

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

Each step operates on well-defined data structures to avoid tight coupling.

---

### Configuration System

The configuration system is split into **three independent stages**:
ConfigTokenizer → ConfigParser → ConfigBuilder
(Lexing) (Syntax) (Runtime objects)

#### 1. ConfigTokenizer

- Converts raw configuration text into typed tokens
- Skips whitespace and comments
- Tracks line and column numbers
- Performs no syntax or semantic validation

#### 2. ConfigParser

- Validates grammar and block structure
- Parses `server` and `location` blocks
- Ensures correct use of braces and semicolons
- Reports syntax errors with precise locations

#### 3. ConfigBuilder

- Builds runtime configuration objects
- Applies default values
- Handles inheritance from `server` to `location`
- Produces final `ServerConfig` and `LocationConfig` structures

At runtime, the webserver only interacts with these built configuration objects.

---

### HTTP Handling

- HTTP requests are parsed into an `HttpRequest` object
- Start line, headers, and body are validated
- Unsupported methods or malformed requests result in appropriate HTTP error codes

The server follows strict HTTP/1.1 rules unless explicitly specified otherwise.

---

### CGI Execution

CGI execution is handled by a dedicated `CGI` class.

Responsibilities:

- Determine whether a request qualifies as CGI
- Prepare CGI environment variables
- Redirect stdin/stdout using pipes
- Execute the CGI interpreter via `execve()`
- Capture and return script output

Supported methods for CGI:

- GET
- POST

The implementation follows the CGI/1.1 specification closely.

---

## Configuration File

The configuration syntax is inspired by Nginx:

```nginx
server {
    listen 8080;
    server_name localhost;

    root ./sites/static;

    location /cgi {
        root ./sites/cgi;
        cgi_pass /usr/bin/python3;
        cgi_ext .py;
    }
}
```

### Error Handling

The server provides custom error pages and standard HTTP responses for common client and server errors:

- Redirection errors (3xx): 301
- Client errors (4xx): 400, 403, 404, 405, 408, 413, 414, 431
* Server errors (5xx): 500
Not all HTTP status codes are implemented due to the project scope.
Additional codes can be added easily if needed.

## Build & Run

```
make
./webserv congiguration/config.conf
./webserv
```
The server will start listening according to the spesified configuration file.
If not specified, the server will start listening according to the default config file(congiguration/simple.conf).

## Testing

- Manual testing with curl
- Browser-based testing
- Custom CGI scripts

### Someideas of testing cases:

- Invalid requests
- Unsupported methods
- Large request bodies
- Invalid configuration files

## Authors

- Server: [Lucio](https://github.com/lsurco-t)
- HTTP & response handling: [Yuxin Wu](https://github.com/Yuxin29)
- Configuration & CGI: [Lin Liu](https://github.com/Lin-0096)
