<TABLE WIDTH="60%" BORDER=0 ALIGN="center">
<TR>
	<TD>
	<H1 ALIGN="center"><?= ucwords(strip_tags($cat)) ?></H1>
	<BR><BR><BR>
	<?
	if (!defined("VALID")) die;

	if (file_exists("cat/$cat.cat.php")) {
		require("cat/$cat.cat.php");
	}
	?>
	</TD>
</TR>
</TABLE>
