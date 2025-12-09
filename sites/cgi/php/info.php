#!/usr/bin/php-cgi
<?php
// CGI script must output HTTP headers
header('Content-Type: text/html; charset=UTF-8');

// Get server variables
$method = $_SERVER['REQUEST_METHOD'] ?? 'UNKNOWN';
$query = $_SERVER['QUERY_STRING'] ?? '';
$remote = $_SERVER['REMOTE_ADDR'] ?? 'unknown';
$script = $_SERVER['SCRIPT_NAME'] ?? '';

// Get current time
$time = date('Y-m-d H:i:s');
$day = date('l');
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP CGI Demo</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            padding: 20px;
        }
        .container {
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            max-width: 700px;
        }
        h1 {
            color: #667eea;
            margin-bottom: 20px;
        }
        h2 {
            color: #333;
            margin-top: 25px;
            margin-bottom: 15px;
        }
        .info {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 5px;
            margin: 15px 0;
        }
        .info p {
            margin: 8px 0;
            color: #333;
        }
        .info strong {
            color: #667eea;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 15px 0;
        }
        th, td {
            padding: 10px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background: #667eea;
            color: white;
        }
        a {
            display: inline-block;
            margin-top: 20px;
            color: #667eea;
            text-decoration: none;
            font-weight: bold;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🐘 PHP CGI Demo</h1>
        <p>This page was dynamically generated using PHP-CGI on the webserver.</p>
        
        <div class="info">
            <h3>Current Server Time:</h3>
            <p><strong><?php echo $day; ?>, <?php echo $time; ?></strong></p>
        </div>

        <h2>Request Details</h2>
        <div class="info">
            <p><strong>Request Method:</strong> <?php echo htmlspecialchars($method); ?></p>
            <p><strong>Script Name:</strong> <?php echo htmlspecialchars($script); ?></p>
            <p><strong>Query String:</strong> <?php echo htmlspecialchars($query) ?: '(none)'; ?></p>
            <p><strong>Remote Address:</strong> <?php echo htmlspecialchars($remote); ?></p>
        </div>

        <h2>PHP Environment</h2>
        <table>
            <tr>
                <th>Variable</th>
                <th>Value</th>
            </tr>
            <tr>
                <td>PHP Version</td>
                <td><?php echo phpversion(); ?></td>
            </tr>
            <tr>
                <td>Server Software</td>
                <td>42 Webserver (Custom)</td>
            </tr>
            <tr>
                <td>CGI Mode</td>
                <td>PHP-CGI</td>
            </tr>
        </table>

        <h2>Dynamic Content Example</h2>
        <div class="info">
            <?php
            $colors = ['red', 'blue', 'green', 'orange', 'purple'];
            $random_color = $colors[array_rand($colors)];
            echo "<p>Random color of the day: <strong style='color: $random_color;'>$random_color</strong></p>";
            
            $random_number = rand(1, 100);
            echo "<p>Random number: <strong>$random_number</strong></p>";
            ?>
        </div>

        <a href="/">← Back to Home</a>
    </div>
</body>
</html>
