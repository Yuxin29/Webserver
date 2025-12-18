#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  CGI POST Request Tests${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Test 1: Simple POST request to form_handler.py
echo -e "${YELLOW}Test 1: Simple POST to /cgi-bin/form_handler.py${NC}"
BODY="username=testuser&email=test@example.com&category=support&message=Hello+from+test"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Content-Length: ${#BODY}\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    echo -ne "${BODY}"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 2: POST with minimal data
echo -e "${YELLOW}Test 2: POST with minimal data${NC}"
BODY="username=user"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Content-Length: ${#BODY}\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    echo -ne "${BODY}"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 3: POST with special characters
echo -e "${YELLOW}Test 3: POST with special characters and spaces${NC}"
BODY="username=John+Doe&email=john.doe%40test.com&message=Special+chars%3A+%26+%3D+%3F"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Content-Length: ${#BODY}\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    echo -ne "${BODY}"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 4: POST with larger payload
echo -e "${YELLOW}Test 4: POST with larger payload${NC}"
MESSAGE=$(python3 -c "print('A' * 500, end='')")
BODY="username=largetest&email=large@test.com&message=${MESSAGE}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Content-Length: ${#BODY}\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    echo -ne "${BODY}"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 5: POST to hello.py (tests POST handling in hello.py)
echo -e "${YELLOW}Test 5: POST to hello.py script${NC}"
BODY="name=POSTTest&data=testing"
{
    echo -ne "POST /cgi-bin/hello.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Content-Length: ${#BODY}\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    echo -ne "${BODY}"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 6: Empty POST body
echo -e "${YELLOW}Test 6: Empty POST body${NC}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Content-Length: 0\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  All POST Tests Completed!${NC}"
echo -e "${BLUE}========================================${NC}"
