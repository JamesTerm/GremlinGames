<?php
$images = array();
$file;
if ($handle = opendir($_SERVER["DOCUMENT_ROOT"].'/images/backgrounds/')) 
{
    while (false !== ($file = readdir($handle))) 
		{
			$pattern="/jpg/";
			if(preg_match($pattern, $file))
			{
				array_push($images, $file);
			}
    }
    closedir($handle);
}
$filename = "/images/backgrounds/".$images[rand(0, count($images)-1)];
echo"
<style type='text/css'>
body{
	background: #000;
	background: #000 url('$filename') top center no-repeat;
}
</style>
";
?>
