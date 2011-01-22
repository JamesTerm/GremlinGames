<?php
// Note that session_start() has to be at the very beginning of the file, before any HTML is sent, or you will get a nasty warning
session_start();
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<title>Gremling Games - Player Info.</title>
</head>

<body>
<p><a href="index.php">Home</a></p>
<H2>Player Information</H2>
<?php

if (!isset($_SESSION['first_name']) || !isset($_SESSION['last_name']) || !isset($_SESSION['email']))
{
	echo '<p><b>Not Logged In</b></p>';
}
else
{
	// All went well, Just show it for now
	echo '<p>First Name: ' . $_SESSION['first_name'] . '</p>';
	echo '<p>Last Name: ' . $_SESSION['last_name'] . '</p>';
	echo '<p>email: ' . $_SESSION['email'] . '</p>';
}

?>
</body>
</html>
