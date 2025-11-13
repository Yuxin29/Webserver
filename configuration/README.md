# Webserver Configuration Guide

## Configuration Files

### Main Configuration: `webserv.conf`
Full-featured configuration with multiple server blocks, CGI support, and various locations.

### Simple Configuration: `simple.conf`
Minimal setup for quick testing with just two servers (static + CGI).

---

## Server Blocks Explained

### 1. Static File Server (Port 8080)

```nginx
server {
    listen 8080;                          # Port number
    server_name localhost www.static.com; # Virtual host names
    
    root ./sites/static;                  # Root directory for files
    index index.html;                     # Default file
    
    client_max_body_size 10M;            # Max upload size
    
    location / {
        allowed_methods GET;              # HTTP methods allowed
        autoindex off;                    # Directory listing on/off
    }
}
```

**Features:**
- Serves static HTML, CSS, JS, images
- Handles GET requests only for root
- No CGI execution
- Custom error pages
- Directory listing (configurable per location)

**Test URLs:**
- `http://localhost:8080/` - Homepage
- `http://localhost:8080/files` - Directory listing example
- `http://localhost:8080/uploads` - File upload area

---

### 2. CGI Server (Port 8081)

```nginx
server {
    listen 8081;
    server_name localhost cgi.local;
    
    root ./sites/cgi;
    index index.html;
    
    client_max_body_size 20M;
    
    # Static files
    location / {
        allowed_methods GET POST;
        autoindex off;
    }
    
    # Python CGI scripts
    location /cgi-bin {
        allowed_methods GET POST;
        root ./sites/cgi/cgi-bin;
        cgi_pass /usr/bin/python3;       # Interpreter path
        cgi_ext .py;                     # File extension
    }
}
```

**Features:**
- Serves both static and dynamic content
- Executes CGI scripts (Python, PHP, Bash)
- Handles GET and POST requests
- Sets CGI environment variables
- Pipes input/output to/from scripts

**Test URLs:**
- `http://localhost:8081/` - CGI server homepage
- `http://localhost:8081/cgi-bin/hello.py` - Basic CGI script
- `http://localhost:8081/cgi-bin/hello.py?name=Alice` - With parameters

---

## Configuration Directives

### Server Level

| Directive | Description | Example |
|-----------|-------------|---------|
| `listen` | Port to bind | `listen 8080;` |
| `server_name` | Virtual host names | `server_name localhost www.example.com;` |
| `root` | Document root directory | `root /var/www/html;` |
| `index` | Default files to serve | `index index.html index.htm;` |
| `client_max_body_size` | Max request body size | `client_max_body_size 10M;` |
| `error_page` | Custom error pages | `error_page 404 /errors/404.html;` |

### Location Level

| Directive | Description | Example |
|-----------|-------------|---------|
| `allowed_methods` | HTTP methods allowed | `allowed_methods GET POST DELETE;` |
| `autoindex` | Directory listing | `autoindex on;` |
| `root` | Override document root | `root /var/www/uploads;` |
| `cgi_pass` | CGI interpreter path | `cgi_pass /usr/bin/python3;` |
| `cgi_ext` | CGI file extension | `cgi_ext .py;` |
| `return` | HTTP redirect | `return 301 /new-page.html;` |

---

## Directory Structure

```
webserver/
├── configuration/
│   ├── webserv.conf          # Full configuration
│   └── simple.conf           # Minimal configuration
├── sites/
│   ├── static/               # Static server files (port 8080)
│   │   ├── index.html
│   │   ├── files/            # Directory listing example
│   │   ├── uploads/          # Upload area
│   │   └── errors/
│   │       ├── 404.html
│   │       └── 50x.html
│   ├── cgi/                  # CGI server files (port 8081)
│   │   ├── index.html
│   │   ├── cgi-bin/          # Python CGI scripts
│   │   │   └── hello.py
│   │   ├── php/              # PHP CGI scripts
│   │   │   └── info.php
│   │   └── scripts/          # Shell scripts
│   │       └── test.sh
│   └── api/                  # API server files (port 9000)
│       └── index.html
```

---

## CGI Environment Variables

When a CGI script is executed, these environment variables are set:

| Variable | Description | Example |
|----------|-------------|---------|
| `REQUEST_METHOD` | HTTP method | `GET`, `POST` |
| `QUERY_STRING` | URL query parameters | `name=Alice&age=25` |
| `CONTENT_TYPE` | Request content type | `application/x-www-form-urlencoded` |
| `CONTENT_LENGTH` | Request body size | `128` |
| `SCRIPT_NAME` | Script path | `/cgi-bin/hello.py` |
| `PATH_INFO` | Extra path info | `/info` |
| `SERVER_NAME` | Server hostname | `localhost` |
| `SERVER_PORT` | Server port | `8081` |
| `SERVER_PROTOCOL` | HTTP version | `HTTP/1.1` |
| `GATEWAY_INTERFACE` | CGI version | `CGI/1.1` |

---

## Testing the Configuration

### Start the Server
```bash
./webserv configuration/simple.conf
```

### Test Static Server
```bash
curl http://localhost:8080/
curl http://localhost:8080/files
```

### Test CGI Server
```bash
# GET request
curl http://localhost:8081/cgi-bin/hello.py?name=World

# POST request
curl -X POST -d "name=Alice&message=Hello" http://localhost:8081/cgi-bin/hello.py
```

### Test Error Pages
```bash
curl http://localhost:8080/nonexistent  # Should return 404
```

---

## Implementation Notes

### For Your WebServer Class:
1. Parse config file → create `Configuration` objects
2. For each `server {}` block → create one `Server` instance
3. Each `Server` binds to its `listen` port
4. Use `poll()` to monitor all server sockets
5. When request arrives → `Http` handler processes it
6. If path matches CGI location → call `CGI` executor

### For Your Http Class:
1. Parse incoming request data
2. Check request path against location blocks
3. If CGI location → check file extension matches `cgi_ext`
4. If match → delegate to `CGI` class with interpreter from `cgi_pass`

### For Your CGI Class:
1. Set environment variables from `Request` object
2. Fork child process
3. Execute script using interpreter from config
4. Pipe request body to stdin (for POST)
5. Capture stdout and parse headers/body
6. Return formatted output

---

## Bonus Features

- Multiple server blocks (virtual hosts)
- Multiple CGI interpreters (Python, PHP, Bash)
- Directory listing
- File uploads (POST)
- File deletion (DELETE)
- Custom error pages
- URL redirections
- Per-location method restrictions

---

## Common Issues

### CGI Script Not Executing
- Check file permissions: `chmod +x hello.py`
- Verify interpreter path: `which python3`
- Check shebang in script: `#!/usr/bin/env python3`

### 404 Not Found
- Verify `root` directive points to correct directory
- Check file exists in specified location
- Ensure paths are relative to server root

### Permission Denied
- Check directory permissions
- Ensure server process can read files
- Verify CGI script is executable

---

Built for 42 Webserv Project 🚀
