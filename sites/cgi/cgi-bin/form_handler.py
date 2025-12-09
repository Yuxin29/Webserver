#!/usr/bin/env python3
import sys
import os
import urllib.parse

# Read POST data from stdin
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
post_data = sys.stdin.read(content_length) if content_length > 0 else ""

# Parse form data
form_data = {}
if post_data:
    for pair in post_data.split('&'):
        if '=' in pair:
            key, value = pair.split('=', 1)
            form_data[urllib.parse.unquote_plus(key)] = urllib.parse.unquote_plus(value)

# Get form fields
username = form_data.get('username', 'Unknown')
email = form_data.get('email', 'Not provided')
category = form_data.get('category', 'general')
message = form_data.get('message', 'No message')

# Output HTTP headers
print("Content-Type: text/html")
print()

# Generate HTML response
html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Form Submission Result</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 20px;
        }}
        .container {{
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            max-width: 700px;
        }}
        h1 {{ color: #667eea; margin-bottom: 20px; }}
        h2 {{ color: #333; margin-top: 25px; margin-bottom: 15px; }}
        .success {{
            background: #d4edda;
            color: #155724;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
            border-left: 4px solid #28a745;
        }}
        .data-box {{
            background: #f8f9fa;
            padding: 20px;
            border-radius: 5px;
            margin: 15px 0;
        }}
        .data-box p {{
            margin: 10px 0;
            color: #333;
        }}
        .data-box strong {{
            color: #667eea;
        }}
        a {{
            display: inline-block;
            margin-top: 20px;
            color: #667eea;
            text-decoration: none;
            font-weight: bold;
        }}
        a:hover {{ text-decoration: underline; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>✅ Form Submitted Successfully!</h1>
        
        <div class="success">
            <strong>Success!</strong> Your form has been processed by the CGI script.
        </div>

        <h2>Submitted Data:</h2>
        <div class="data-box">
            <p><strong>Username:</strong> {username}</p>
            <p><strong>Email:</strong> {email}</p>
            <p><strong>Category:</strong> {category}</p>
            <p><strong>Message:</strong></p>
            <p style="white-space: pre-wrap; padding: 10px; background: white; border-radius: 3px;">{message}</p>
        </div>

        <h2>Request Details:</h2>
        <div class="data-box">
            <p><strong>Method:</strong> {os.environ.get('REQUEST_METHOD', 'UNKNOWN')}</p>
            <p><strong>Content Length:</strong> {content_length} bytes</p>
            <p><strong>Script Name:</strong> {os.environ.get('SCRIPT_NAME', 'unknown')}</p>
        </div>

        <p>In a real application, this data would be saved to a database, sent via email, or processed according to business logic.</p>

        <a href="/forms/demo.html">← Submit Another Form</a>
        <a href="/" style="margin-left: 20px;">Home</a>
    </div>
</body>
</html>"""

print(html)
