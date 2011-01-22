		<div id="header">
			<a href="/"><img id="headerImage" src="/images/elements/rimspace_logo2.png"/></a>
			<br/>
			<?php 
				if($user->data['is_registered'])
				{
					//logout link.
					echo '<p style="float: right;">';
					echo '<a href="' . append_sid("/forum/ucp.$phpEx", 'mode=logout&redirect=/', true, $user->session_id). '">Logout [ '.$user->data['username'].' ]</a></p>';
				}
				else
				{
					//login form.
					echo '<form method="POST" action="/forum/ucp.php?mode=login"><label for="username">ID: <input type="text" name="username" id="username" size="40"><label for="password">Password: <input type="password" name="password" id="password" size="40"><input type="checkbox" name="autologin" checked="false" style="display: none;"><input type="submit" value="Submit" name="login" id="login"></p><input type="hidden" name="redirect" value="/"></form>';
				}
			?>
		</div>
