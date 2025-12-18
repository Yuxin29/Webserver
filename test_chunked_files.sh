#!/bin/bash

# Test chunked encoding with webserv.conf (POST to /files)
{
    echo -ne "POST /files HTTP/1.1\r\n"
    echo -ne "Host: localhost:8080\r\n"
    echo -ne "Transfer-Encoding: chunked\r\n"
    echo -ne "\r\n"
    
    # Send chunk 1: 1000 bytes
    echo -ne "3e8\r\n"
    python3 -c "print('A' * 1000, end='')"
    echo -ne "\r\n"
    
    # Send chunk 2: 2000 bytes
    echo -ne "7d0\r\n"
    python3 -c "print('B' * 2000, end='')"
    echo -ne "\r\n"
    
    # Send chunk 3: 5000 bytes
    echo -ne "1388\r\n"
    python3 -c "print('C' * 5000, end='')"
    echo -ne "\r\n"
    
    # Send terminating chunk
    echo -ne "0\r\n"
    echo -ne "\r\n"
} | nc localhost 8080
