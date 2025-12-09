#!/usr/bin/env python3
"""
Simple GET-only CGI test script
Tests query parameter parsing and environment variables
"""

import os
from urllib.parse import parse_qs

# CGI Header - MUST come first
print("Content-Type: text/html\r")
print("\r")

# Get query string
query_string = os.environ.get('QUERY_STRING', '')
params = parse_qs(query_string) if query_string else {}

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>GET Test - CGI</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }")
print("        .container { background: white; padding: 30px; border-radius: 8px; max-width: 600px; margin: 0 auto; }")
print("        h1 { color: #2196F3; }")
print("        .success { background: #d4edda; padding: 15px; border-left: 4px solid #28a745; margin: 15px 0; }")
print("        .info { background: #e8f4f8; padding: 15px; margin: 15px 0; border-left: 4px solid #2196F3; }")
print("        code { background: #f5f5f5; padding: 2px 6px; border-radius: 3px; color: #e83e8c; }")
print("        .param { margin: 10px 0; }")
print("        a { color: #2196F3; text-decoration: none; }")
print("        a:hover { text-decoration: underline; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>✅ GET Request Test</h1>")

# Success message
print("        <div class='success'>")
print("            <strong>CGI Script Executed Successfully!</strong><br>")
print(f"            Method: <code>GET</code><br>")
print(f"            Query String: <code>{query_string if query_string else '(empty)'}</code>")
print("        </div>")

# Display parsed parameters
if params:
    print("        <h2>📋 Parsed Parameters:</h2>")
    print("        <div class='info'>")
    for key, values in params.items():
        print(f"            <div class='param'>")
        print(f"                <strong>{key}:</strong> <code>{values[0]}</code>")
        print(f"            </div>")
    print("        </div>")
    
    # Special handling for 'name' parameter
    if 'name' in params:
        print(f"        <h2>👋 Hello, {params['name'][0]}!</h2>")
else:
    print("        <div class='info'>")
    print("            <strong>No parameters provided.</strong> Try adding query parameters to the URL.")
    print("        </div>")

# Environment variables
print("        <h2>🔧 CGI Environment:</h2>")
print("        <div class='info'>")
env_vars = ['REQUEST_METHOD', 'QUERY_STRING', 'SERVER_PROTOCOL', 'SCRIPT_NAME', 'SERVER_NAME']
for var in env_vars:
    value = os.environ.get(var, 'Not set')
    print(f"            <code>{var}</code> = {value}<br>")
print("        </div>")

# Test links
print("        <h2>🧪 Test These URLs:</h2>")
print("        <ul>")
print("            <li><a href='/cgi-bin/test_get.py'>No parameters</a></li>")
print("            <li><a href='/cgi-bin/test_get.py?name=Alice'>Single parameter</a></li>")
print("            <li><a href='/cgi-bin/test_get.py?name=Bob&age=30'>Multiple parameters</a></li>")
print("            <li><a href='/cgi-bin/test_get.py?name=Charlie&age=25&city=Paris'>Many parameters</a></li>")
print("            <li><a href='/cgi-bin/test_get.py?message=Hello%20World'>URL encoded</a></li>")
print("        </ul>")

print("        <p style='margin-top: 30px; text-align: center; color: #999;'>")
print("            <a href='/'>Back to Home</a>")
print("        </p>")

print("    </div>")
print("</body>")
print("</html>")
