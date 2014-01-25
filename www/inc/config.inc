<?
	define("VALID", 1);

	$dbHost = "localhost";
	$dbUser = "root";
	$dbPass = "";
	$dbDatabase = "netmond";

	$urlPrefix = "http://";

	$admin_ips = array(
	);

	if (in_array($_SERVER['REMOTE_ADDR'], $admin_ips)) {
		define("ADMIN", 1);
	}

	$str_not_allowed = "<H1 ALIGN=\"center\">Access denied</H2>";
?>
