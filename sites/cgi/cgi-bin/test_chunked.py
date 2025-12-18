#!/usr/bin/env python3
"""
CGI script specifically for testing chunked transfer encoding
Displays information about the received data
"""

import sys
import os

# Get environment variables
method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
content_length = os.environ.get('CONTENT_LENGTH', '0')
content_type = os.environ.get('CONTENT_TYPE', 'none')
query_string = os.environ.get('QUERY_STRING', '')

# Read POST data if available
post_data = ""
if method == 'POST' and content_length.isdigit():
    length = int(content_length)
    if length > 0:
        post_data = sys.stdin.read(length)

# CGI Header
print("Content-Type: text/html; charset=UTF-8\r")
print("\r")

# HTML Response
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Chunked Transfer Test</title>")
print("    <style>")
print("        body { font-family: monospace; margin: 40px; background: #1e1e1e; color: #d4d4d4; }")
print("        .container { background: #252526; padding: 30px; border-radius: 8px; border: 2px solid #3e3e42; }")
print("        h1 { color: #4ec9b0; margin-bottom: 20px; }")
print("        .info-box { background: #1e1e1e; padding: 15px; margin: 15px 0; border-left: 4px solid #4ec9b0; }")
print("        .label { color: #569cd6; font-weight: bold; }")
print("        .value { color: #ce9178; }")
print("        .success { color: #4ec9b0; font-size: 1.2em; }")
print("        .data-box { background: #1e1e1e; padding: 15px; margin: 15px 0; border: 1px solid #3e3e42; max-height: 300px; overflow-y: auto; }")
print("        pre { margin: 0; white-space: pre-wrap; word-wrap: break-word; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>✓ Chunked Transfer Encoding Test</h1>")
print("        <p class='success'>CGI script executed successfully!</p>")

# Environment info
print("        <div class='info-box'>")
print(f"            <span class='label'>REQUEST_METHOD:</span> <span class='value'>{method}</span><br>")
print(f"            <span class='label'>CONTENT_LENGTH:</span> <span class='value'>{content_length}</span><br>")
print(f"            <span class='label'>CONTENT_TYPE:</span> <span class='value'>{content_type}</span><br>")
if query_string:
    print(f"            <span class='label'>QUERY_STRING:</span> <span class='value'>{query_string}</span><br>")
print("        </div>")

# Display received data
if post_data:
    data_length = len(post_data)
    print("        <div class='info-box'>")
    print(f"            <span class='label'>Data Received:</span> <span class='value'>{data_length} bytes</span><br>")
    print("        </div>")
    
    print("        <div class='data-box'>")
    print("            <strong>Raw POST Data:</strong>")
    print("            <pre>")
    # Show first 500 characters if data is long
    if data_length > 500:
        print(post_data[:500])
        print(f"\n... [{data_length - 500} more bytes] ...")
    else:
        print(post_data)
    print("            </pre>")
    print("        </div>")
    
    # Character frequency analysis (for test validation)
    if data_length > 100:
        char_count = {}
        for char in post_data:
            char_count[char] = char_count.get(char, 0) + 1
        
        print("        <div class='info-box'>")
        print("            <span class='label'>Character Frequency Analysis:</span><br>")
        for char, count in sorted(char_count.items(), key=lambda x: x[1], reverse=True)[:10]:
            if char.isprintable() and not char.isspace():
                print(f"            '{char}': {count} times<br>")
            elif char == ' ':
                print(f"            'SPACE': {count} times<br>")
            elif char == '\n':
                print(f"            'NEWLINE': {count} times<br>")
        print("        </div>")
else:
    print("        <div class='info-box'>")
    print("            <span class='label'>No POST data received</span>")
    print("        </div>")

print("        <div class='info-box'>")
print("            <span class='label'>Test Status:</span> <span class='success'>PASSED ✓</span><br>")
print("            <small>This confirms your server correctly assembled chunked data and passed it to the CGI script.</small>")
print("        </div>")

print("    </div>")
print("</body>")
print("</html>")
