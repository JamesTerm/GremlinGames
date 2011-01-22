<?php

require('config.php');

class PasswordHasher
{
	public static function Hash($strToHash, $withPrefix = true)
	{
		if ($withPrefix)
			$hashedStr = sha1(HASH_PREFIX . $strToHash);
		else
			$hashedStr = sha1($strToHash);
		return $hashedStr;
	}
}

?>
