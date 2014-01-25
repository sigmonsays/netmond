<?
	if (!defined("VALID")) die;

	$mysqlConnection = mysql_connect($dbHost, $dbUser, $dbPass);
	if (!$mysqlConnection) die(mysql_error());

	if (!mysql_select_db($dbDatabase, $mysqlConnection)) die(mysql_error());

?>
