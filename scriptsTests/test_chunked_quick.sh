#!/bin/bash

# Quick tester specifically for test_chunked.py CGI script
# This validates chunked encoding is properly handled

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  Chunked Encoding Test - test_chunked.py${NC}"
echo -e "${BLUE}================================================${NC}\n"

echo -e "${YELLOW}Sending chunked POST to /cgi-bin/test_chunked.py${NC}"
echo -e "${YELLOW}This will send 3 chunks and verify data assembly${NC}\n"

{
    echo -ne "POST /cgi-bin/test_chunked.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Chunk 1: 1000 'A' characters
    echo -ne "3e8\r\n"
    python3 -c "print('A' * 1000, end='')"
    echo -ne "\r\n"
    
    # Chunk 2: 2000 'B' characters
    echo -ne "7d0\r\n"
    python3 -c "print('B' * 2000, end='')"
    echo -ne "\r\n"
    
    # Chunk 3: 3000 'C' characters
    echo -ne "bb8\r\n"
    python3 -c "print('C' * 3000, end='')"
    echo -ne "\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Check the HTML output above!${NC}"
echo -e "${GREEN}Expected: 6000 total bytes received${NC}"
echo -e "${GREEN}  - 1000 'A' characters${NC}"
echo -e "${GREEN}  - 2000 'B' characters${NC}"
echo -e "${GREEN}  - 3000 'C' characters${NC}"
echo -e "${GREEN}================================================${NC}"
