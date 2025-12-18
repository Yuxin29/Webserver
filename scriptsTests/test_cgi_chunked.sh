#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  CGI Chunked Transfer-Encoding Tests${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Test 1: Small chunks to form_handler.py
echo -e "${YELLOW}Test 1: Chunked POST to /cgi-bin/form_handler.py (3 small chunks)${NC}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Chunk 1: username field
    CHUNK1="username=chunked"
    echo -ne "$(printf '%x' ${#CHUNK1})\r\n"
    echo -ne "${CHUNK1}\r\n"
    
    # Chunk 2: email field
    CHUNK2="&email=chunk@test.com"
    echo -ne "$(printf '%x' ${#CHUNK2})\r\n"
    echo -ne "${CHUNK2}\r\n"
    
    # Chunk 3: message field
    CHUNK3="&message=Sent+via+chunked+encoding"
    echo -ne "$(printf '%x' ${#CHUNK3})\r\n"
    echo -ne "${CHUNK3}\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 2: Larger chunks with form data
echo -e "${YELLOW}Test 2: Chunked POST with larger payload (1KB per chunk)${NC}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Chunk 1: 1000 bytes
    CHUNK1=$(python3 -c "print('username=bigdata&message=' + 'A' * 978, end='')")
    echo -ne "$(printf '%x' ${#CHUNK1})\r\n"
    echo -ne "${CHUNK1}\r\n"
    
    # Chunk 2: 500 bytes
    CHUNK2=$(python3 -c "print('B' * 500, end='')")
    echo -ne "$(printf '%x' ${#CHUNK2})\r\n"
    echo -ne "${CHUNK2}\r\n"
    
    # Chunk 3: Final part
    CHUNK3="&email=chunked@large.com"
    echo -ne "$(printf '%x' ${#CHUNK3})\r\n"
    echo -ne "${CHUNK3}\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 3: Single large chunk
echo -e "${YELLOW}Test 3: Single large chunk (5KB)${NC}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Single 5KB chunk
    CHUNK=$(python3 -c "print('username=singleChunk&message=' + 'X' * 4973, end='')")
    echo -ne "$(printf '%x' ${#CHUNK})\r\n"
    echo -ne "${CHUNK}\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 4: Many small chunks (stress test)
echo -e "${YELLOW}Test 4: Many small chunks (10 chunks of ~100 bytes each)${NC}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Send 10 small chunks
    for i in {1..10}; do
        CHUNK=$(python3 -c "print('chunk${i}=' + 'D' * 90 + '&', end='')")
        echo -ne "$(printf '%x' ${#CHUNK})\r\n"
        echo -ne "${CHUNK}\r\n"
    done
    
    # Final chunk with username
    CHUNK="username=manyChunks"
    echo -ne "$(printf '%x' ${#CHUNK})\r\n"
    echo -ne "${CHUNK}\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 5: Chunked to hello.py with query string
echo -e "${YELLOW}Test 5: Chunked POST to hello.py with query parameters${NC}"
{
    echo -ne "POST /cgi-bin/hello.py?source=chunked HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Chunk 1
    CHUNK1="data=chunked"
    echo -ne "$(printf '%x' ${#CHUNK1})\r\n"
    echo -ne "${CHUNK1}\r\n"
    
    # Chunk 2
    CHUNK2="&test=true"
    echo -ne "$(printf '%x' ${#CHUNK2})\r\n"
    echo -ne "${CHUNK2}\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

sleep 1

# Test 6: Very large chunked transfer (testing limits)
echo -e "${YELLOW}Test 6: Large chunked transfer - 3 chunks of 5KB each (15KB total)${NC}"
{
    echo -ne "POST /cgi-bin/form_handler.py HTTP/1.1\r\n"
    echo -ne "Host: localhost:8081\r\n"
    echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "Connection: close\r\n"
    echo -ne "\r\n"
    
    # Chunk 1: 5KB
    echo -ne "1388\r\n"
    python3 -c "print('E' * 5000, end='')"
    echo -ne "\r\n"
    
    # Chunk 2: 5KB
    echo -ne "1388\r\n"
    python3 -c "print('F' * 5000, end='')"
    echo -ne "\r\n"
    
    # Chunk 3: 5KB with form data
    CHUNK3=$(python3 -c "print('username=largechunk&message=' + 'G' * 4965, end='')")
    echo -ne "$(printf '%x' ${#CHUNK3})\r\n"
    echo -ne "${CHUNK3}\r\n"
    
    # Terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8081
echo -e "\n${GREEN}-----------------------------------${NC}\n"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  All Chunked Tests Completed!${NC}"
echo -e "${BLUE}========================================${NC}"
