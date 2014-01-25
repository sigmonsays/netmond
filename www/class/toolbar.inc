<?
	class Toolbar {
		var $toolbarHeadings;
		var $toolbars;
		var $toolbarTitle;

		/* constructor */
		function Toolbar($mainTitle) {
			$this->toolbarTitle = $mainTitle;
			$this->reset();
		}

		function reset() {
			$this->toolbars = array();
			$this->toolbarHeadings = array();
		}

		function addHeading($title) {
			$this->toolbarHeadings[] = $title;
			$c = count($this->toolbarHeadings);
			return $c - 1;
		}

		function addLink($headingID, $title, $link) {
			$this->toolbars[$headingID][] = array($title, $link);
		}

		function display() {
			$c = count($this->toolbars);
			echo "<TABLE WIDTH=\"100%\" CELLPADDING=0>\n";
			echo "<TR><TD STYLE=\"border-bottom: 1px solid black;\"><FONT SIZE=\"+2\">" . $this->toolbarTitle . "</FONT></TD></TR>\n";
			echo "<TR><TD>&nbsp;</TD></TR>\n";
			for($i=0; $i<=$c; $i++) {
				echo "<TR><TD><FONT SIZE=\"+1\">" . $this->toolbarHeadings[$i] . "</FONT></TD></TR>\n";
				$entries = $this->toolbars[$i];
				$c2 = count($entries);
				$color = "#e5e5e5";
				for($j=0; $j<$c2; $j++) {
					list($title, $url) = $entries[$j];
					$color = (($color == "e5e5e5") ? "#e2e2e2" : "#e5e5e5");
					echo "<TR><TD BGCOLOR=\"$color\">- &nbsp; <A HREF=\"$url\">$title</A></TD></TR>\n";
				}
				echo "<TR><TD>&nbsp;</TD></TR>\n";
			}
			echo "</TABLE>\n";
		}
	}
?>
