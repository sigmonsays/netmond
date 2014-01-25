<?
	require("inc/config.inc");
	require("inc/functions.inc");
	require("inc/mysql.inc");

	$cat = $_GET['cat'];

	if (empty($cat)) $cat = "status";

	if (!$nocontent) {
		require("inc/header.inc");
		?>
		<TABLE WIDTH="100%" BORDER=0 ALIGN="center" CELLPADDING=0 CELLSPACING=0>
		<TR>
			<TD VALIGN="top" ALIGN="left" WIDTH=100>
			<? require("inc/toolbar.inc"); ?>
			</TD>

			<TD VALIGN="top" ALIGN="center" WIDTH=100%>
		<?
	}

	require("inc/main.inc");

	if (!$nocontent) {
		?>
			</TD>
		</TR>
		</TABLE>
		<BR>
		<?
		require("inc/footer.inc");
	}
	
?>
