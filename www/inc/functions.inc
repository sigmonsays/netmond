<?
	function redir($url) {
		if (headers_sent()) {
			echo '<SCRIPT LANGUAGE="javascript">';
			printf("document.location='%s';\n", $url);
			echo '</SCRIPT>';
			die;
		} else {
			header("Location: $url");
		}
	}
?>
