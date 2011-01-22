<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<title>Test Hasher</title>
</head>

<body>
<p>This file tests the hashing function.</p>

<?php

if (isset ($_GET['to_be_hashed']))
{
	require('password_hasher.php');
	
	$original_string = $_GET['to_be_hashed'];
	
	echo '<p>The Hash of [' . $original_string . '] is: </p>';
	echo PasswordHasher::Hash($original_string, false);
	echo '<br /><br />';
	
	echo '<p>... And the Hash of [' . HASH_PREFIX . $original_string . '] is: </p>';
	echo PasswordHasher::Hash($original_string, true);
	echo '<br /><br />';
}
?>

<form action="test_hasher.php">
	Write your password:
	<input type="text" name="to_be_hashed" /><br />
	<input type="submit" value="Hash It!" />
</form>

</body>
</html>
