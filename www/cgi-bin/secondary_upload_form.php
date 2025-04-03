<?php
// Set the content type for the response
$content_type = "text/html";

// Output the content type header
header("Content-Type: $content_type");
echo "\r\n";  // Body separator

// HTML structure for the response
echo "<html>";
echo "<head><title>File Upload</title></head>";
echo "<body>";

// Check if the file was uploaded successfully
if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_FILES['file'])) {
    $file = $_FILES['file'];

    // Check for upload errors
    if ($file['error'] === UPLOAD_ERR_OK) {
        $tmp_name = $file['tmp_name'];
        $name = basename($file['name']);
        $target_file = 'uploads/' . $name;

        // Check the file type (for example, allow only images)
        $allowed_types = ['image/jpeg', 'image/png', 'image/gif'];
        if (in_array($file['type'], $allowed_types)) {
            // Move the file to the desired directory
            if (move_uploaded_file($tmp_name, $target_file)) {
                echo "<h1>File '$name' uploaded successfully!</h1>";
                // Meta redirect after successful upload
                echo '<meta http-equiv="refresh" content="2;url=/upload_done.html">';
            } else {
                echo "<h1>Error: Failed to move uploaded file.</h1>";
            }
        } else {
            echo "<h1>Error: Invalid file type. Only JPEG, PNG, and GIF are allowed.</h1>";
        }
    } else {
        echo "<h1>Error: " . get_upload_error_message($file['error']) . "</h1>";
    }
} else {
    echo "<h1>Error: No file uploaded</h1>";
}

// HTML footer
echo "</body>";
echo "</html>";

// Function to map error codes to user-friendly messages
function get_upload_error_message($error_code) {
    switch ($error_code) {
        case UPLOAD_ERR_OK:
            return "There is no error, the file uploaded successfully.";
        case UPLOAD_ERR_INI_SIZE:
            return "The uploaded file exceeds the upload_max_filesize directive in php.ini.";
        case UPLOAD_ERR_FORM_SIZE:
            return "The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.";
        case UPLOAD_ERR_PARTIAL:
            return "The uploaded file was only partially uploaded.";
        case UPLOAD_ERR_NO_FILE:
            return "No file was uploaded.";
        case UPLOAD_ERR_NO_TMP_DIR:
            return "Missing a temporary folder.";
        case UPLOAD_ERR_CANT_WRITE:
            return "Failed to write file to disk.";
        case UPLOAD_ERR_EXTENSION:
            return "A PHP extension stopped the file upload.";
        default:
            return "Unknown error.";
    }
}
?>

