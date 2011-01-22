<?php include($_SERVER['DOCUMENT_ROOT']."/includes/phpBBIntegration.php");?>
<?php $currentPage = "/gallery/concepts"; ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<head>
	<?php include($_SERVER['DOCUMENT_ROOT'].'/backgroundRandomizer.php');?>
	<link rel="stylesheet" type="text/css" href="/css/wrapper.css"/>
	<link rel="stylesheet" type="text/css" href="/css/header.css"/>
	<link rel="stylesheet" type="text/css" href="/css/navigation.css"/>
	<link rel="stylesheet" type="text/css" href="/css/mainContent.css"/>
	<link rel="stylesheet" type="text/css" href="/css/columns.css"/>
	<link rel="stylesheet" type="text/css" href="/css/footer.css"/>
	<link rel="stylesheet" type="text/css" href="/css/gallery.css"/>
	<link rel="stylesheet" type="text/css" href="/css/general.css"/>
	<script type="text/javascript" src="/js/gallery.js">
	</script>

</head>
<body>
	<div id="wrapper">
		<?php include($_SERVER['DOCUMENT_ROOT']."/includes/header.php");?>
		<?php include($_SERVER['DOCUMENT_ROOT']."/includes/topNav.php");?>
		<?php include($_SERVER['DOCUMENT_ROOT']."/includes/mainContent.php");?>
		<div id="currentImage"></div>
		<div id="thumbs">
		<?php
			$images = array();
			$file;
			if ($handle = opendir($_SERVER["DOCUMENT_ROOT"].'/images/gallery/misc/thumbs/')) 
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
			for($i = 0; $i < sizeof($images); $i++)
			{
				echo"<img src='/images/gallery/misc/thumbs/".$images[$i]."' onclick=\"load('misc', '".$images[$i]."')\"/>\n";
			}
			?>

		</div>
		</div> <!-- mainContent closed -->
		<?php include($_SERVER['DOCUMENT_ROOT']."/includes/footer.php");?>
	</div>
</body>
</html>