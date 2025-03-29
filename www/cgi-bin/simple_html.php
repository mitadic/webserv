#!/usr/bin/php-cgi
<?php
// Read input from STDIN
$input = file_get_contents("php://stdin");

// Output HTTP headers and HTML content
echo "Content-Type: text/html\n\n";
echo "<html><head><title>CGI PHP Script</title></head><body>";
echo "<h1>PHP CGI Script Output</h1>";
echo "<pre>" . htmlspecialchars($input) . "</pre>";
echo "</body></html>";
?>
