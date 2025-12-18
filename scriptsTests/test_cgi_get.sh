#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  CGI GET Request Tests${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Test 1: Simple GET request to hello.py (no parameters)
echo -e "${YELLOW}Test 1: Simple GET to /cgi-bin/hello.py${NC}"
{
    echo -ne "GET /cgi-bin/hello.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 2: GET request with query string (single parameter)
echo -e "${YELLOW}Test 2: GET with query string - /cgi-bin/test_get.py?name=WebServ${NC}"
{
    echo -ne "GET /cgi-bin/test_get.py?name=WebServ HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 3: GET request with multiple parameters
echo -e "${YELLOW}Test 3: GET with multiple parameters - /cgi-bin/test_get.py?name=User&id=42&status=active${NC}"
{
    echo -ne "GET /cgi-bin/test_get.py?name=User&id=42&status=active HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 4: GET request with special characters (URL encoded)
echo -e "${YELLOW}Test 4: GET with URL encoded params - /cgi-bin/hello.py?name=John+Doe&message=Hello%20World${NC}"
{
    echo -ne "GET /cgi-bin/hello.py?name=John+Doe&message=Hello%20World HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 5: GET request with empty query string
echo -e "${YELLOW}Test 5: GET with empty query string - /cgi-bin/test_get.py?${NC}"
{
    echo -ne "GET /cgi-bin/test_get.py? HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  All GET Tests Completed!${NC}"
echo -e "${BLUE}========================================${NC}"
