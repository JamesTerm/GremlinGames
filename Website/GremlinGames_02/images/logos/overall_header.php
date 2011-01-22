<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" dir="{S_CONTENT_DIRECTION}" lang="{S_USER_LANG}" xml:lang="{S_USER_LANG}">
<head>
<!-- INCLUDE ca_config.html -->
<meta http-equiv="content-type" content="text/html; charset={S_CONTENT_ENCODING}" />
<meta http-equiv="content-language" content="{S_USER_LANG}" />
<meta http-equiv="content-style-type" content="text/css" />
<meta http-equiv="imagetoolbar" content="no" />
<meta name="resource-type" content="document" />
<meta name="distribution" content="global" />
<meta name="copyright" content="2000, 2002, 2005, 2007 phpBB Group" />
<meta name="keywords" content="" />
<meta name="description" content="" />
{META}
<title>{SITENAME} - <!-- IF S_IN_MCP -->{L_MCP} - <!-- ELSEIF S_IN_UCP -->{L_UCP} - <!-- ENDIF -->{PAGE_TITLE}</title>

<link rel="stylesheet" href="{T_STYLESHEET_LINK}" type="text/css" />
<!--[if IE]>
<link rel="stylesheet" type="text/css" href="{T_THEME_PATH}/ie7.css" />
<![endif]-->
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
<script type="text/javascript">
// <![CDATA[

	var onload_functions = new Array();
	var onunload_functions = new Array();
	var ca_main_width = '{$CA_WIDTH}';

<!-- IF S_USER_PM_POPUP -->
	if ({S_NEW_PM})
	{
		popup('{UA_POPUP_PM}', 400, 225, '_phpbbprivmsg');
	}
<!-- ENDIF -->

function popup(url, width, height, name)
{
	if (!name)
	{
		name = '_popup';
	}

	window.open(url.replace(/&amp;/g, '&'), name, 'height=' + height + ',resizable=yes,scrollbars=yes,width=' + width);
	return false;
}

function jumpto()
{
	var page = prompt('{LA_JUMP_PAGE}:', '{ON_PAGE}');
	var per_page = '{PER_PAGE}';
	var base_url = '{A_BASE_URL}';

	if (page !== null && !isNaN(page) && page == Math.floor(page) && page > 0)
	{
		if (base_url.indexOf('?') == -1)
		{
			document.location.href = base_url + '?start=' + ((page - 1) * per_page);
		}
		else
		{
			document.location.href = base_url.replace(/&amp;/g, '&') + '&start=' + ((page - 1) * per_page);
		}
	}
}

/**
* Find a member
*/
function find_username(url)
{
	popup(url, 760, 570, '_usersearch');
	return false;
}

/**
* Mark/unmark checklist
* id = ID of parent container, name = name prefix, state = state [true/false]
*/
function marklist(id, name, state)
{
	var parent = document.getElementById(id);
	if (!parent)
	{
		eval('parent = document.' + id);
	}

	if (!parent)
	{
		return;
	}

	var rb = parent.getElementsByTagName('input');
	
	for (var r = 0; r < rb.length; r++)
	{
		if (rb[r].name.substr(0, name.length) == name)
		{
			rb[r].checked = state;
		}
	}
}

<!-- IF ._file -->

	/**
	* Play quicktime file by determining it's width/height
	* from the displayed rectangle area
	*
	* Only defined if there is a file block present.
	*/
	function play_qt_file(obj)
	{
		var rectangle = obj.GetRectangle();

		if (rectangle)
		{
			rectangle = rectangle.split(',')
			var x1 = parseInt(rectangle[0]);
			var x2 = parseInt(rectangle[2]);
			var y1 = parseInt(rectangle[1]);
			var y2 = parseInt(rectangle[3]);

			var width = (x1 < 0) ? (x1 * -1) + x2 : x2 - x1;
			var height = (y1 < 0) ? (y1 * -1) + y2 : y2 - y1;
		}
		else
		{
			var width = 200;
			var height = 0;
		}

		obj.width = width;
		obj.height = height + 16;

		obj.SetControllerVisible(true);

		obj.Play();
	}
<!-- ENDIF -->

	/**
	* New function for handling multiple calls to window.onload and window.unload by pentapenguin
	*/
	window.onload = function()
	{
		for (var i = 0; i < onload_functions.length; i++)
		{
			eval(onload_functions[i]);
		}
	}

	window.onunload = function()
	{
		for (var i = 0; i < onunload_functions.length; i++)
		{
			eval(onunload_functions[i]);
		}
	}


if(navigator.userAgent && navigator.userAgent.indexOf('Mac OS X') > 0)
{
	document.write('<link rel="stylesheet" href="{T_THEME_PATH}/mac.css" type="text/css" />');
}

// ]]>
</script>
<script type="text/javascript" src="{T_TEMPLATE_PATH}/ca_scripts.js"></script>
</head>
<body class="{S_CONTENT_DIRECTION}">

<a name="top"></a>

<table border="0" cellspacing="0" cellpadding="0" width="{$CA_WIDTH}" id="maintable" align="center">
<tr>
	<td id="logorow" align="center"><div id="logo-left"><div id="logo-right">
		<a href="{U_INDEX}">{SITE_LOGO_IMG}</a>
	</div></div></td>
</tr>
<tr>
	<td class="navrow">
		<!-- IF not S_IS_BOT -->
			<!-- IF S_USER_LOGGED_IN --><a href="{U_PROFILE}">{L_PROFILE}</a> &#8226; <!-- ENDIF -->
			<!-- IF S_USER_LOGGED_IN -->
				<!-- IF S_DISPLAY_PM --><a href="{U_PRIVATEMSGS}">{PRIVATE_MESSAGE_INFO}</a> &#8226; <!-- ENDIF -->
			<!-- ELSEIF S_REGISTER_ENABLED --><a href="{U_REGISTER}">{L_REGISTER}</a> &#8226; <!-- ENDIF -->
		<!-- ENDIF -->
		<!-- IF U_RESTORE_PERMISSIONS --><a href="{U_RESTORE_PERMISSIONS}">{L_RESTORE_PERMISSIONS}</a> &#8226; <!-- ENDIF -->
		<a href="{U_FAQ}">{L_FAQ}</a>
		<!-- IF S_DISPLAY_SEARCH --> &#8226; <a href="{U_SEARCH}">{L_SEARCH}</a><!-- ENDIF -->
		<!-- IF not S_IS_BOT -->
			<!-- IF S_DISPLAY_MEMBERLIST --> &#8226; <a href="{U_MEMBERLIST}">{L_MEMBERLIST}</a><!-- ENDIF -->
		<!-- ENDIF -->
		<!-- IF not S_IS_BOT --> &#8226; <a href="{U_LOGIN_LOGOUT}">{L_LOGIN_LOGOUT}</a><!-- ENDIF -->
	</td>
</tr>
<tr>
	<td id="contentrow">

<!-- IF not S_IS_BOT && not $CA_SKIP_PM_NOTIFY && S_USER_LOGGED_IN && S_DISPLAY_PM -->
	<!-- IF S_USER_NEW_PRIVMSG -->
		<div class="pm-notify-new"><div class="pm-notify-header">{L_PRIVATE_MESSAGES}</div><a href="{U_PRIVATEMSGS}">{PRIVATE_MESSAGE_INFO}<!-- IF PRIVATE_MESSAGE_INFO_UNREAD -->, {PRIVATE_MESSAGE_INFO_UNREAD}<!-- ENDIF --></a></div>
	<!-- ELSEIF S_USER_UNREAD_PRIVMSG -->
		<div class="pm-notify-unread"><div class="pm-notify-header">{L_PRIVATE_MESSAGES}</div><a href="{U_PRIVATEMSGS}">{PRIVATE_MESSAGE_INFO}<!-- IF PRIVATE_MESSAGE_INFO_UNREAD -->, {PRIVATE_MESSAGE_INFO_UNREAD}<!-- ENDIF --></a></div>
	<!-- ENDIF -->
<!-- ENDIF -->  

<!-- IF S_BOARD_DISABLED and S_USER_LOGGED_IN and (U_MCP or U_ACP) -->
	<div class="pm-notify-new"><div class="pm-notify-header">{L_INFORMATION}</div>{L_BOARD_DISABLED}</div>
<!-- ENDIF -->

    <table width="100%" cellspacing="0">
    <tr>
        <td class="gensmall">
            <!-- IF S_USER_LOGGED_IN -->{LAST_VISIT_DATE}<br /><!-- ENDIF -->
            <!-- IF S_DISPLAY_SEARCH --><a href="{U_SEARCH_UNANSWERED}">{L_SEARCH_UNANSWERED}</a> | <a href="{U_SEARCH_ACTIVE_TOPICS}">{L_SEARCH_ACTIVE_TOPICS}</a><!-- ENDIF -->
        </td>
        <td class="gensmall" align="{S_CONTENT_FLOW_END}">
            {CURRENT_TIME}<br />
            <!-- IF S_DISPLAY_SEARCH and S_USER_LOGGED_IN --><a href="{U_SEARCH_NEW}">{L_SEARCH_NEW}</a> | <a href="{U_SEARCH_SELF}">{L_SEARCH_SELF}</a><!-- ENDIF -->
        </td>
    </tr>
    </table>

	<!-- INCLUDE breadcrumbs.html -->

	<br />

