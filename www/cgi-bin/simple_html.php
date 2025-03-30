#!/usr/bin/php-cgi
<?php
// Set HTTP headers
echo "Content-Type: text/html\n\n";

// Read raw POST body (this works for any content type)
$input = file_get_contents("php://input");

// Output the captured input inside an HTML page
echo "<html><head><title>CGI PHP Script</title></head><body>";
echo "<h1>PHP CGI Script Output</h1>";
echo "<pre>" . htmlspecialchars($input) . "</pre>";
echo "</body></html>";
?>
