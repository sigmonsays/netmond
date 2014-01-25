<?
	if (!defined("VALID")) die;

	$sql = mysql_query("SELECT * FROM servers ORDER BY `group` ASC, id ASC", $mysqlConnection);
	if (!$sql) die(mysql_error());
	$group = -1;
	@$fd = fsockopen("localhost", 2421);
	if ($fd) {
		fputs($fd, "status\n");
		$moo = "";
		while ($poo = fgets($fd)) $moo .= $poo;
		if ($moo) {
			echo '<TABLE WIDTH="50%" STYLE="border: 1px solid black" BGCOLOR="white">';
			echo "<TR><TD><B>netmond</B></TD></TR>\n";
			echo "<TR><TD><PRE>\n";
			echo $moo;
			echo "</PRE></TD></TR>";
		echo '</TABLE>';
		}
	}

	while ($server = mysql_fetch_object($sql)) {

		$ports = split(",", $server->ports);
		$sql_status = mysql_query("SELECT * FROM status WHERE serverid='$server->id'", $mysqlConnection);
		$status = array();
		if ($sql_status && mysql_num_rows($sql_status)) {
			while ($tmp = mysql_fetch_object($sql_status)) {
				$status[$tmp->port] = $tmp->status;
			}
		} else {
			$status[] = 0;
		}
		if (($group != $server->group) && $server->group) {
			$qry = "SELECT name FROM groups WHERE id='$server->group'";
			$sql_group = mysql_query($qry, $mysqlConnection);
			if ($sql_group && mysql_num_rows($sql_group)) {
				list($poo) = mysql_fetch_row($sql_group);
				echo "<H3>$poo</H3>\n";
			}
			$group = $server->group;

		}
		if ($group == -1) {
			echo "<H3><I>Not grouped</I></H3>\n";
			$group = 0;
		}
		echo "<TABLE BORDER=0 ALIGN=\"left\" WIDTH=\"100%\" CELLSPACING=5 CELLPADDING=0>\n";
			$c = count($ports);
			echo "<TR>";
			echo "<TD NOWRAP>";
				echo "<FONT SIZE=+1 TITLE=\"$server->host\">$server->name</FONT>\n";
			echo "</TD><TD>&nbsp; &nbsp;</TD>\n";
			for($i=0; $i<$c; $i++) {
				$name = strtoupper(getservbyport($ports[$i], "tcp"));
				if (empty($name)) $name = $ports[$i];
				$portNumber = $ports[$i];

				if ($status[$portNumber] == 0) {
					$statusColor = "#3df429";

				} else if ($status[$portNumber] == 1) {
					$statusColor = "#f4f129";

				} else if ($status[$portNumber] >= 2) {
					$statusColor = "red";
				}
				
				echo "<TD TITLE=\"$portNumber\" HEIGHT=1 WIDTH=100 VALIGN=\"middle\" ALIGN=\"CENTER\" STYLE=\"border: 1px solid black;\" WIDTH=150 BGCOLOR=\"$statusColor\"><B>$name</B></TD>";

			}
			echo "</TR>";
		echo "</TABLE><BR><BR><BR>\n";
	}
?>
