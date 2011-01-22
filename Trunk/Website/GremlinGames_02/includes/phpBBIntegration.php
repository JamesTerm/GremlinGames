<?php
	define('IN_PHPBB', true);
	$phpbb_root_path = $_SERVER['DOCUMENT_ROOT']."/forum/";
	$phpEx = substr(strrchr(__FILE__, '.'), 1);
	include($phpbb_root_path . 'common.' . $phpEx);

	// Start session management
	$user->session_begin();
	$auth->acl($user->data);
	$user->setup(); 
	echo"<?xml version = '1.0' encoding = 'utf-8'?>"
?>
