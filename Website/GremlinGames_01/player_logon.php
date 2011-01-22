<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<title>Gremlin Games - Player Logon</title>
	<script language="javascript" type="text/javascript">
	<!--
	var emailValid = false;
	var firstNameValid = false;
	var lastNameValid = false;
	var passwordValid = false;
	
		function ValidateEmail(control, trim)
		{
			// Get the trimmed version of what the user typed for verification
			var email = control.value.replace(/^\s+|\s+$/g, '');
			if (trim)
				control.value=email;
				
			// Validate
			emailValid = true;
			
			// Check for empty or too long
			if ((email.length == 0) || (email.length > 60))
				emailValid = false;
				
			// TODO: other email validation here
			
			// See if this is ok
			TryEnableSubmit();
		}
		
		function ValidateFirstName(control, trim)
		{
			// Get the trimmed version of what the user typed for verification
			var firstName = control.value.replace(/^\s+|\s+$/g, '');
			if (trim)
				control.value=firstName;
				
			// Validate
			firstNameValid = true;
			
			// Check for empty
			if ((firstName.length == 0) || (firstName.length > 60))
				firstNameValid = false;
				
			// TODO: other first name validation here
			
			// See if this is ok
			TryEnableSubmit();
		}
		
		function ValidateLastName(control, trim)
		{
			// Get the trimmed version of what the user typed for verification
			var lastName = control.value.replace(/^\s+|\s+$/g, '');
			if (trim)
				control.value=lastName;
				
			// Validate
			lastNameValid = true;
			
			// Check for empty
			if ((lastName.length == 0) || (lastName.length > 60))
				lastNameValid = false;
				
			// TODO: other last name validation here
			
			// See if this is ok
			TryEnableSubmit();
		}
		
		function ValidatePassword(control1, control2, trim)
		{
			// Get the trimmed version of what the user typed for verification
			var password = control1.value.replace(/^\s+|\s+$/g, '');
			if (trim)
				control1.value=password;
				
			var password2 = control2.value.replace(/^\s+|\s+$/g, '');
			if (trim)
				control2.value=password2;
				
			// Validate
			passwordValid = (password==password2);
			
			// Check for empty
			if ((password.length == 0) || (password.length > 30))
				passwordValid = false;
				
			// TODO: other first name validation here
			
			// See if this is ok
			TryEnableSubmit();
		}
		
		function ResetButtonClick()
		{
			// Reset makes all of the values empty, so they are no longer valid
			emailValid = passwordValid = lastNameValid = firstNameValid = false;
			TryEnableSubmit();
		}
		
		function TryEnableSubmit()
		{
			// Only enable the submit button if all the other stuff is valid
			var enabled = emailValid && passwordValid && lastNameValid && firstNameValid;
			document.forms.RegisterForm.SubmitButton.disabled = !enabled;
		}
		
	-->
	</script>
</head>

<body>
<p><a href="index.php">Home</a></p>

<!-- Watch for error codes from the register_check.php !-->
<?php
if (isset ($_GET['error_code']))
{
	$err_code = $_GET['error_code'];
	switch ($err_code)
	{
	case missing_first_name:
		$errStr = 'Missing First Name';
		break;
	case invalid_first_name:
		$errStr = 'Invalid First Name';
		break;
	case missing_last_name:
		$errStr = 'Missing Last Name';
		break;
	case invalid_last_name:
		$errStr = 'Invalid Last Name';
		break;
	case missing_email:
		$errStr = 'Missing Email';
		break;
	case invalid_email:
		$errStr = 'Invalid Email Address';
		break;
	case missing_password:
		$errStr = 'Missing Password';
		break;
	case invalid_password:
		$errStr = 'Invalid Password';
		break;
	case email_exists:
		$errStr = 'That email address already exists in our Player Database';
		break;
	case incorrect_password:
		$errStr = 'Incorrect Password';
		break;
	case no_user:
		$errStr = 'That email address does not exist in our Player Database';
		break;
	default:
		$errStr = 'Unknown Error Code [' . $err_code . ']';
	}
	echo '<p><h3>Registration Failed: ' . $errStr . '</p></h3>';
}
?>

<H2>Log On</H2>
<form action="logon_check.php" method="post" name="LogonForm">
	Email: <input type="text" name="email" /><br />
	Password: <input type="password" name="password" /><br />
	<input type="submit" value="Logon" name="SubmitButton" />
</form>
<hr>
<H2>Register</H2>
<form action="register_check.php" method="post" name="RegisterForm">
	Email: <input type="text" name="email" onkeypress="ValidateEmail(this, false)" 
				onchange="ValidateEmail(this, true)" /><br />
	Password: <input type="password" name="password" onkeypress="ValidatePassword(this, document.forms.RegisterForm.password2, false)" 
				onchange="ValidatePassword(this, document.forms.RegisterForm.password2, true)" /><br />
	Re-Type Password: <input type="password" name="password2" onkeypress="ValidatePassword(this, document.forms.RegisterForm.password, false)" 
				onchange="ValidatePassword(this, document.forms.RegisterForm.password, true)" /><br />
	<br />
	First Name: <input type="text" name="first_name" onkeypress="ValidateFirstName(this, false)" 
				onchange="ValidateFirstName(this, true)" /><br />
	Last Name: <input type="text" name="last_name" onkeypress="ValidateLastName(this, false)" 
				onchange="ValidateLastName(this, true)" /><br />
	
	<input type="submit" value="Register" name="SubmitButton" disabled />
	<input type="reset" value="Reset" onclick="ResetButtonClick()" />
</form>
</body>
</html>
