<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<title>Gremlin Games Database DEV Generator</title>
</head>
<body>
<p><a href="index.php">Home</a></p>

<?php
// Developer's should use this file to allow everyone to update their database tables to reflect current
// development.  When doing an SVN update, check to see if this file has been modified and run it if it has.

require('sql_connect.php');
	
// Use this command to drop tables only if necessary
// mysql_query("DROP TABLE player_table") or die('Failed to drop player_table. ' . mysql_error());
// echo "player_table dropped OK<br>";

// The SQL query string to Create the table we need
$sql_makeTable =
  "CREATE TABLE IF NOT EXISTS player_table (
	player_id int(11) NOT NULL auto_increment,
	first_name varchar(64) NOT NULL,
	last_name varchar(64) NOT NULL,
	email varchar(64) NOT NULL,
	password varchar(64) NOT NULL,
	PRIMARY KEY(player_id),
	UNIQUE KEY idx_email (email)
  )";

// Run the query and report problems
mysql_query($sql_makeTable) or die('Failed to create player_table. ' . mysql_error());
echo "player_table generated OK<br>";


// Let us know that all is done
echo "All database operations complete<br>";
 

?>


</body>
</html>
