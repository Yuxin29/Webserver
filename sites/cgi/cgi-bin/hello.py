#!/usr/bin/env python3
"""
Simple CGI script example for webserver testing
Demonstrates GET and POST handling
"""

import os
import sys
from urllib.parse import parse_qs

# CGI Header - MUST come first
print("Content-Type: text/html; charset=UTF-8\r")
print("\r")

# Get request method
method = os.environ.get('REQUEST_METHOD', 'GET')

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>CGI Test - Hello World</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }")
print("        .container { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }")
print("        h1 { color: #333; }")
print("        .info { background: #e8f4f8; padding: 10px; margin: 10px 0; border-left: 4px solid #2196F3; }")
print("        code { background: #f5f5f5; padding: 2px 6px; border-radius: 3px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1> CGI Script Working!</h1>")

# Display request method
print(f"        <div class='info'>")
print(f"            <strong>Request Method:</strong> <code>{method}</code>")
print(f"        </div>")

# Handle GET parameters
if method == 'GET':
    query_string = os.environ.get('QUERY_STRING', '')
    if query_string:
        params = parse_qs(query_string)
        print("        <div class='info'>")
        print("            <strong>GET Parameters:</strong><br>")
        for key, value in params.items():
            print(f"            <code>{key}</code> = <code>{value[0]}</code><br>")
        print("        </div>")
        
        # Personalized greeting if 'name' parameter exists
        if 'name' in params:
            print(f"        <h2>Hello, {params['name'][0]}! 👋</h2>")

# Handle POST data
elif method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print("        <div class='info'>")
        print("            <strong>POST Data Received:</strong><br>")
        print(f"            <code>{post_data}</code>")
        print("        </div>")

# Display some environment variables
print("        <h3>Environment Variables:</h3>")
print("        <div class='info'>")
env_vars = ['SERVER_SOFTWARE', 'SERVER_NAME', 'GATEWAY_INTERFACE', 'SERVER_PROTOCOL', 
            'REQUEST_METHOD', 'PATH_INFO', 'SCRIPT_NAME', 'QUERY_STRING']
for var in env_vars:
    value = os.environ.get(var, 'Not set')
    print(f"            <code>{var}</code> = {value}<br>")
print("        </div>")

print("        <h3>Try These URLs:</h3>")
print("        <ul>")
print("            <li><a href='/cgi-bin/hello.py'>Basic request</a></li>")
print("            <li><a href='/cgi-bin/hello.py?name=Alice'>With name parameter</a></li>")
print("            <li><a href='/cgi-bin/hello.py?name=Bob&age=25'>Multiple parameters</a></li>")
print("        </ul>")

print("    </div>")
print("</body>")
print("</html>")
