<?php
session_start();
/* 
Does the server side check for the registration.  Has PHP code for validating the information.  Creates the table entry for the newly registered user.  Sets current session information for the player.  Redirects to #player_info.php to show the appropriate data, OR redirects back to player_logon.php with an error message if there was a problem.
*/

// Check for missing parameters
if (!isset($_POST['first_name']))
{
	// Reload the player_logon page with an error code that says we were missing the first_name
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=missing_first_name">';
	exit();
}
if (!isset($_POST['last_name']))
{
	// Reload the player_logon page with an error code that says we were missing the last_name
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=missing_last_name">';
	exit();
}
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

$first_name = trim($_POST['first_name']);
$last_name = trim($_POST['last_name']);
$email = trim($_POST['email']);
$password = trim($_POST['password']);

$first_name_len = strlen($first_name);
$last_name_len = strlen($last_name);
$email_len = strlen($email);
$password_len = strlen($password);

if (($first_name_len == 0) || ($first_name_len > 60))
{
	// Reload the player_logon page with an error code that says we have an invalid first_name
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=invalid_ first_name">';
	exit();
}
if (($last_name_len == 0) || ($last_name_len > 60))
{
	// Reload the player_logon page with an error code that says we have an invalid last_name
	echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=invalid_last_name">';
	exit();
}
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

// TODO: Further validate the Password
// TODO: Further validate the email

// We made it this far, so we can do the database stuff
require('sql_connect.php');

// Make sure the email does not already exist
$sql_query_player = "SELECT player_id FROM player_table WHERE email = '$email'";
$existing_result = mysql_query($sql_query_player) or die("Failed to check for email " . mysql_error());

// Is there a better way to check for one existing?
while ($existing_rows = mysql_fetch_assoc($existing_result))
{
	if (sizeof($existing_rows) != 0)
	{
		// Reload the player_logon page with an error code that says we have an existing player
		echo '<meta http-equiv="REFRESH" content="0;url=player_logon.php?error_code=email_exists">';
		exit();
	}
}

// We want to hash the password
require('password_hasher.php');
$hashed_password = PasswordHasher::Hash($password);

// Check to make sure the email does not already exist.
$sql_add_player = "INSERT INTO player_table (email, password, first_name, last_name) VALUES ('$email', '$hashed_password', '$first_name', '$last_name')";

// Do the add, if this fails, we tell the user
// TODO: Would it be better to do the database check first, rather than letting the failure di it for us?
mysql_query($sql_add_player) or die("Failed to add Player " . mysql_error());

// All Went well so far,  Place the information in the Session for later retrieval
$_SESSION['first_name'] = $first_name;
$_SESSION['last_name'] = $last_name;
$_SESSION['email'] = $email;

// Re-direct to plyer-info page
echo '<meta http-equiv="REFRESH" content="0;url=player_info.php">';
exit();

?>
