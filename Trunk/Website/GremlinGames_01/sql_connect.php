<?php
// If you need to make a change in the structure, try to keep existing data as much as possible.  This file
// should be created so that it can be run multiple times without an issue.  
require('config.php');

// make the general connection to mysql
$conn = mysql_connect(SQL_HOST, SQL_USER, SQL_PASS) 
	or die('Could not connect to MySQL. ' . mysql_error());

// Connect specifically to our database
mysql_select_db(SQL_DB, $conn)
	or die('Could not connect to our database (' . SQL_DB . '). ' . mysql_error());
	
?>
