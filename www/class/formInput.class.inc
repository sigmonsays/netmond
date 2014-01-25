<?
	class formInput {
		/* make all values keys or vice versa */
		function array_smear($a, $v2k = true) {
			$values = ($v2k) ? array_values($a) : array_keys($a);

			$a = array();
			foreach ($values as $v)
				$a[$v] = $v;
			return ($a);
		}

		/* convert epoch timestamp to sql format */
		function epoch2sql($timestamp) {
			return date("YmdHis", $timestamp);
		}

		/* convert mySQL timestamp (field type timestamp) to epoch */
		function sql2epoch($timestamp) {
			//example time 20010627110324
			$year=substr($timestamp, 0, 4);
			$month=substr($timestamp, 4, 2);
			$day=substr($timestamp, 6,2);
			$hour=substr($timestamp, 8, 2);
			$minute=substr($timestamp, 10,2);
			$second=substr($timestamp, 12,2);
			return mktime($hour, $minute, $second, $month, $day, $year);
	        }

		function select($name, $selections = array(), $selected = "") {
			echo "<SELECT NAME=\"$name\">\n";
			while (@list($k, $v) = each($selections)) { 
				if (empty($sel)) {
					if ($selected == $v || $selected == $k) $sel = "SELECTED";
				} else $sel = "";
				echo "<OPTION $sel VALUE=\"" . stripslashes($k) . "\">$v</OPTION>\n";
			}
			echo "</SELECT>\n";
		}

		function hidden($name, $value) {
			echo "<INPUT TYPE=\"hidden\" NAME=\"$name\" VALUE=\"" . htmlspecialchars(stripslashes($value)) . "\">\n";
		}

		function password($name) {
			echo "<INPUT TYPE=\"password\" NAME=\"$name\">\n";
		}

		function radio($name, $value = "", $checked = 0) {
			if ($checked) $check = "CHECKED"; else $check = "";
			echo "<INPUT TYPE=\"radio\" $check NAME=\"$name\" VALUE=\"$value\">\n";
		}

		function submit($value = "Submit") {
			echo "<INPUT TYPE=\"submit\" VALUE=\"$value\">\n";
		}

		function boolean($name, $yesno = 1) {
			if ($yesno == 1) $check = "CHECKED"; else $check = "";
			echo "<INPUT $check TYPE=\"radio\" NAME=\"$name\" VALUE=1> Yes &nbsp; ";

			if ($yesno == 0) $check = "CHECKED"; else $check = "";
			echo "<INPUT $check TYPE=\"radio\" NAME=\"$name\" VALUE=0> No\n";
		}

		function checkbox($name, $value = "", $checked = 0) {
			if ($checked) $check = "CHECKED"; else $check = "";
			echo "<INPUT TYPE=\"checkbox\" $check NAME=\"$name\" VALUE=\"" . htmlspecialchars(stripslashes($value)) . "\">\n";
		}

		function file($name) {
			echo "<INPUT TYPE=\"file\" NAME=\"$name\">\n";
		}

		function fileList($name, $directory, $selected_file = "", $max_files) {
			if (empty($directory) || !is_dir($directory)) return 0;

			$patterns = func_get_args();
			$patternCount = count($patterns);

			$d = opendir($directory);
			if (!$d) return 0;

			$i = 0;
			readdir($d);
			readdir($d);
			$files = array();
			while ($file = readdir($d)) {
				if (++$i > $max_files && $max_files != 0) break;

				$ext = strrchr($file, ".");
				if ($patternCount > 3) {
					if (in_array($ext, $patterns)) {
						$files[$file] = $file;
					}
				} else {
					$files[$file] = $file;
				}
			}
			closedir($d);
			$this->select($name, $files, $selected_file);
		}

		function text($name, $value = "", $size = 0, $extra = "") {
			if ($size) $extra .= " SIZE=\"$size\"";
			echo "<INPUT $extra TYPE=\"text\" NAME=\"$name\" VALUE=\"" . htmlspecialchars(stripslashes($value)) . "\">\n";
		}

		function textarea($name, $rows = 10, $cols = 10, $value = "") {
			echo "<TEXTAREA NAME=\"$name\" ROWS=\"$rows\" COLS=\"$cols\">" . htmlspecialchars(stripslashes($value)) . "</TEXTAREA>\n";
		}

		function date($name, $format, $value = 0, $timestamp_format = "epoch") {

			if ($timestamp_format != "sql-timestamp" && $timestamp_format != "epoch") {
				echo "<I>WARNING: \$timestamp_format is neither 'epoch' or 'sql-timestamp'!</I><BR>";
				return 0;
			}
		
			if ($timestamp_format == "sql-timestamp" && $value != 0) {
				$value = $this->sql2epoch($value);
			}

			$i = 0;

			if (intval($value) == 0)
				list($hour, $hour24, $minute, $second, $month, $day, $year, $day_week, $days_in_month) = split(",", date("h,H,i,s,m,j,Y,w,t"));
			else
				list($hour, $hour24, $minute, $second, $month, $day, $year, $day_week, $days_in_month) = split(",", date("h,H,i,s,m,j,Y,w,t", $value));

			while (($x = substr($format, $i++, 1)) != "") {
				if(!$open_tag){
					if ($x=='a' || $x=='A') { //am or pm
						$val_sel = ($hour24 <= 12) ? 0 : 1;
						$val = ($x == 'a') ? array("am", "pm") : array("AM", "PM");
						$this->select($name . '[ampm]', $val, $val_sel);

					} elseif ($x == 'B') { //Swatch Internet time 

					} elseif ($x == 'd') { //day of the month, 2 digits with leading zeros; i.e. "01" to "31"
						if ($day < 10) {
							$val = "0" . $day;
						} else {
							$val = $day;
						}
						$this->text($name . '[day]', $val, 2);

					} elseif ($x == 'D') { //day of the week, textual, 3 letters; e.g. "Fri" 
						$day_list = array("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat");
		                	        $day_sel = date("w", mktime(1, 1, 1, $month, $day, $year));
        			                $this->select($name.'[dayofweek]', $day_list, $day_sel);

					} elseif ($x == 'F') { // month (long textual) -- Januarary
		                	        $months_list = array();
		        	                for($j=1; $j<=12; $j++) {
	        		                        $months_list[$j] = date("F", mktime(1, 1, 1, $j));
		                	        }
        			                $this->select($name . '[month]', $months_list, $month);

			                } elseif ($x =='g') { //hour, 12-hour format without leading zeros; i.e. "1" to "12" 
						$this->text($name . '[hour]',intval($hour), 2);

					} elseif ($x == 'G') { //hour, 24-hour format without leading zeros; i.e. "0" to "23" 
						$this->text($name. '[hour]', intval($hour24), 2);

					} elseif ($x == 'h') { // hour (12 hour format) with leading zero
		        	                $this->text($name . '[hour]', $hour, 2);

			                } elseif ($x == 'H') { // hour (24 hour format) with leading zero
		                	        $this->text($name . '[hour]', $hour24, 2);

		        	        } elseif ($x == 'i') { // i.e. "00" to "59" 
						$this->text($name . '[minute]', $minute, 3);

					} elseif ($x=='I'){ //"1" if Daylight Savings Time, "0" otherwise. 
						//dont need in a selector

					} elseif ($x == 'j') { // day of the month without leading zeros; i.e. "1" to "31"
		                	        $this->text($name . '[day]', $month, 3);

		        	        } elseif ($x=='l'){ //day of the week, textual, long; e.g. "Friday"
						$day_list = array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");
		                	        $day_sel = date("w", mktime(1, 1, 1, $month, $day,$year));
						$this->select($name . '[dayofweek]', $day_list, $day_sel);

					} elseif ($x=='L'){ //boolean for whether it is a leap year; i.e. "0" or "1" 
						// dont think need in selection

					} elseif ($x == 'm') { // month (numerical)
	                        		$this->text($name . '[month]', $month, 3);

		                	} elseif ($x == 'M') { // month (abbrev) -- Jan
        			                $months_list = array();
	                        		for($j=1; $j<=12; $j++) {
		                	                $months_list[$j] = date("M", mktime(1, 1, 1, $j));
        			                }
	                        		$this->select($name . '[month]', $months_list, $month);

		                	} elseif ($x=='n') { //month without leading zeros; i.e. "1" to "12" 
						$this->text($name.'[month]',intval($month),3);

					} elseif ($x=='O') { //Difference to Greenwich time in hours; e.g. "+0200" 
						//?!

					} elseif ($x=='r') { //RFC 822 formatted date; e.g. "Thu, 21 Dec 2000 16:01:07 +0200" (added in PHP 4.0.4) 
						//?!

					} elseif ($x == 's') { // seconds; i.e. "00" to "59" 
						$this->text($name . '[second]', $second, 3);

					} elseif ($x=='S') { //English ordinal suffix for the day of the month, 2 characters; i.e. "st", "nd", "rd" or "th" 
						//?!

					} elseif ($x=='t') { //number of days in the given month; i.e. "28" to "31"
						//?!

					} elseif ($x=='T') { //Timezone setting of this machine; e.g. "EST" or "MDT" 
						//fixme

					} elseif ($x=='U') { //seconds since the Unix Epoch (January 1 1970 00:00:00 GMT) 
						//?!

					} elseif ($x=='w') { //day of the week, numeric, i.e. "0" (Sunday) to "6" (Saturday) 
						$this->text($name . '[dayofweek]', date("w", mktime(1, 1, 1, $month, $day, $year)), 2);

					} elseif ($x=='W'){ //ISO-8601 week number of year, weeks starting on Monday (added in PHP 4.1.0) 
						$this->text($name . '[weeknumber]', date("W", mktime(1, 1, 1, $month, $day, $year)), 3);

					} elseif ($x == 'y') { //year, 4 digits; e.g. "1999" 
						$this->text($name . '[year]', $year, 3);

					} elseif ($x == 'Y') { //year, 2 digits; e.g. "99" 
						$this->text($name . '[year]', $year, 5);

					} elseif ($x=='z') { //day of the year; i.e. "0" to "365" 
						$this->text($name . '[daynumber]', date("z", mktime(1, 1, 1, $month, $day, $year)), 3);

					} elseif ($x=='Z') {
						// timezone offset in seconds (i.e. "-43200" to "43200"). The offset for timezones 
						// west of UTC is always negative, and for those east of UTC is always positive. 

					} elseif ($x=='<') {
						$tag_open=1;
						echo $x;
					} else {
						echo $x;
					}
				}else{
					if($x==">"){
						$tag_open=0;
					}
					echo $x;
				}
			}
		}

	}
?>
