<?php
session_start();
/* 
Does the server side check for the logon.  Has PHP code for validating the information.  Checks the hashed password against what is in the DB.  Sets current session information for the player.  Redirects to #player_info.php to show the appropriate data, OR redirects back to player_logon.php with an error message if there was a problem.
*/

// Check for missing parameters
if (!isset($_POST['email']))
{
	// Reload the player_logon page with an error code that says we were missing the email
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=missing_email">';
	exit();
}
if (!isset($_POST['password']))
{
	// Reload the player_logon page with an error code that says we were missing the password
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=missing_password">';
	exit();
}

$email = trim($_POST['email']);
$password_in = trim($_POST['password']);

$email_len = strlen($email);
$password_len = strlen($password_in);

if (($email_len == 0) || ($email_len > 60))
{
	// Reload the player_logon page with an error code that says we have an invalid email
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=invalid_email">';
	exit();
}
if (($password_len == 0) || ($password_len > 30))
{
	// Reload the player_logon page with an error code that says we have an invalid password
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=invalid_password">';
	exit();
}

// We want to hash the password
require('password_hasher.php');
$hashed_password = PasswordHasher::Hash($password_in);

// We made it this far, so we can do the database stuff
require('sql_connect.php');

// Make sure the email does not already exist
$sql_query_player = "SELECT password, first_name, last_name FROM player_table WHERE email = '$email'";
$results = mysql_query($sql_query_player) or die("Failed to check for email " . mysql_error());

// Is there a better way to check for one existing?
while ($rows = mysql_fetch_array($results))
{
	extract($rows);
	if ($password == $hashed_password)
	{
		// All Went well so far,  Place the information in the Session for later retrieval
		$_SESSION['first_name'] = $first_name;
		$_SESSION['last_name'] = $last_name;
		$_SESSION['email'] = $email;

		// Re-direct to plyer-info page
		echo '<meta http-equiv="REFRESH" content="0;url=player_info.php">';
		exit();
	}
	else
	{
		// Incorrect Password, Tell the user about it
		echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=incorrect_password">';
		exit();
	}
}

// If we got this far, there are no users in the DB with that email
echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=no_user">';
exit();

?>
