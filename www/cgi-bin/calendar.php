#!/usr/bin/env php
<?php
// Output header
header("Content-Type: text/html");

// Get current date info
$month = isset($_POST['month']) ? (int)$_POST['month'] : date("n");
$year  = isset($_POST['year']) ? (int)$_POST['year'] : date("Y");
if ($month < 1 || $month > 12) $month = date("n");
if ($year < 1900 || $year > 2100) $year = date("Y");
$today = date("j");
$current_month = date("n");
$current_year = date("Y");

// First day of the month
$first_day_timestamp = mktime(0, 0, 0, $month, 1, $year);
$days_in_month = date("t", $first_day_timestamp);  // Total days in month
$start_day = date("w", $first_day_timestamp);  // 0 = Sunday, 6 = Saturday

// Weekday names
$weekdays = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];

// Output HTML
echo "<!DOCTYPE html>";
echo "<html><head><title>PHP Calendar</title>";
echo "<style>
        table { border-collapse: collapse; font-family: sans-serif; }
        td, th { padding: 8px; border: 1px solid #ccc; text-align: center; }
        .today { background: #ffeb3b; font-weight: bold; }
      </style>";
echo "</head><body>";
echo "<h1>Calendar for " . date("F", mktime(0, 0, 0, $month, 1)) . " " . $year . "</h1>";

echo "<table>";
echo "<tr>";
foreach ($weekdays as $wd) {
    echo "<th>$wd</th>";
}
echo "</tr><tr>";

// Add blank cells before the first day
for ($i = 0; $i < $start_day; $i++) {
    echo "<td></td>";
}

// Fill in the days
for ($day_num = 1; $day_num <= $days_in_month; $day_num++) {
    $is_today = ($day_num == $today && $month == $current_month && $year == $current_year);
	$current_class = $is_today ? " class='today'" : "";
    echo "<td$current_class>$day_num</td>";

    // New row after Saturday
    if ((($day_num + $start_day) % 7) == 0) {
        echo "</tr><tr>";
    }
}

// Fill in the remaining blank cells of the last week
$remaining = (7 - (($days_in_month + $start_day) % 7)) % 7;
for ($i = 0; $i < $remaining; $i++) {
    echo "<td></td>";
}
echo "</tr></table>";

echo "</body></html>";
?>
