<?
	class sqlForm extends formInput {
		var $mysqlConnection, $table, $database;
		var $fields;
		var $fieldTypes;
		var $fieldArgs;
		var $fieldFuncs;
		var $urlPrefix;
		var $fieldCount;
		var $mode;

		var $id, $value; /* temp holding spot for current unique id and value */

		var $uniqueField, $actionVariable;

		var $sqlOrderBy, $sqlStart, $sqlOffset, $sqlWhere;

		var $urlVars;
		var $version;

		var $rowColors;
		var $cellpadding;

		/* constructor */
		function sqlForm($mysqlConnection, $table, $database) {
			$this->horizontal_row_spacing = 0;
			$this->vertical_row_spacing = 0;

			$this->mysqlConnection = $mysqlConnection;
			$this->table = $table;
			$this->database = $database;
			$this->actionVariable = "action";
			$this->urlVars = "";

			$this->rowColors = "#FFFFFF,#e8e8e8";
			$this->cellpadding = 4;

			list($major, $minor, $revision) = split("\.", phpversion());
			$this->version = intval($major . $minor . $revision);

			if (!$mysqlConnection) return -1;

			if (!empty($database)) {
				if (!mysql_select_db($database, $mysqlConnection)) return -1;
			}

			/* list fields */
			$fieldList = mysql_list_fields($database, $table, $mysqlConnection);
			$columns = mysql_num_fields($fieldList);
			for ($i = 0; $i < $columns; $i++) {
				$this->fields[$i] = mysql_field_name($fieldList, $i);
				$this->fieldTypes[$i] = "text";
				$this->fieldArgs[$i] = array();
				$this->fieldFuncs[$i] = "";
				$this->fieldFuncArgs[$i] = array();
			}
			$this->fieldCount = $columns;
			$this->uniqueField = "id";
			$this->sqlStart = 0;
			$this->sqlOffset = 30;
			$this->mode = "list,add,edit,delete";
		}


		/* this function performs a translation, mostly it's a dirty hack for supporting date() from formInput.class ... 
		   however I could possibly see it being useful in the future for more translations .... */

		function translate_value($fieldName, $fieldValue) {
			$fieldID = $this->fieldIDFromName($fieldName);
			$fieldType = $this->fieldTypes[$fieldID];
			$func_args = $this->fieldArgs[$fieldID];

			switch ($fieldType) {
				case "date":
					if (is_array($fieldValue)) {

						/* we need to put missing values in this array -- hour, minute, sec, etc */
						if (empty($fieldValue['hour'])) $fieldValue['hour'] = 1;
						if (empty($fieldValue['minute'])) $fieldValue['minute'] = 1;
						if (empty($fieldValue['second'])) $fieldValue['second'] = 1;
						if (empty($fieldValue['month'])) $fieldValue['month'] = 1;
						if (empty($fieldValue['day'])) $fieldValue['day'] = 1;
						if (empty($fieldValue['year'])) $fieldValue['year'] = 1;

						$newFieldValue = mktime($fieldValue['hour'], $fieldValue['minute'], $fieldValue['second'], $fieldValue['month'], 
									$fieldValue['day'], $fieldValue['year']);

						/* translate the field value to the appropriate timestamp format */
						if ($func_args[3] == "sql-timestamp") {
							$newFieldValue = $this->epoch2sql($newFieldValue);
						}
						return $newFieldValue;
					} else {
						/* we don't know what to do here
							the variable they gave us wasn't an associative array like we expected, so lets just return the value
							untouched. We dont' even need this else besides for this comment =) 
						*/
						echo "WARNING: Unexpected variable type \$fieldValue in translate_value(..)!<BR>";
						return $fieldValue;
					}
					break;
				

				default: // no translation needed, return the value untouched
					return $fieldValue;
					break;
			}
		}

		function setFieldFunction($fieldName, $function, $type = "user-defined") {
			$fieldID = $this->fieldIDFromName($fieldName);
			$this->fieldFuncs[$fieldID] = ($type == "user-defined") ? $function : "built-in:$function";
			$func_args = func_get_args();
			$this->fieldFuncArgs[$function] = $func_args;
		}

		function formInput($fieldName, $fieldValue = "") {
			$fieldType = $this->getFieldType($fieldName);
			$fieldID = $this->fieldIDFromName($fieldName);

			$func_args = $this->fieldArgs[$fieldID];

			switch ($fieldType) {

				case "select":
					$this->select($fieldName, $func_args[2], $fieldValue);
					break;

				case "password":
					$this->password($fieldName);
					break;

				case "radio":
					$this->radio($fieldName, $fieldValue, $func_args[2]);
					break;

				case "submit":
					$this->submit($fieldName);
					break;

				case "boolean":
					$this->boolean($fieldName, $this->value);
					break;

				case "checkbox":
					$this->checkbox($fieldName, $func_args[2], intval($fieldValue));
					break;

				case "file":
					$this->file($fieldName);
					break;

				default:
					echo "<I><SMALL>Assuming input type text</SMALL></I><BR>";
				case "text":
					$this->text($fieldName, $fieldValue, $func_args[2]);
					break;

				case "textarea":
					$this->textarea($fieldName, $func_args[2], $func_args[3], $fieldValue);
					break;

				case "date":
					/* perform a simple translation on $fieldValue, it's by default an associative array, we need to 
					   make it a unix timestamp */
					if (is_array($fieldValue)) {
						if ($this->version < 410) global $HTTP_POST_VARS;
						$post_vars = ($this->version < 410) ? $HTTP_POST_VARS : $_POST;

						extract($post_vars[$fieldName], "EXTR_PREFIX_ALL", "date");
						$fieldValue = mktime($date_hour, $date_minute, $date_second, $date_month, $date_day, $date_year);
					}
					$this->date($fieldName, $func_args[2], $fieldValue, $func_args[3]);
					break;
			}
		}

		function getFieldType($fieldName) {
			return $this->fieldTypes[$this->fieldIDFromName($fieldName)];
		}

		function setFieldType($field, $type) {
			$fieldID = $this->fieldIDFromName($field);

			$this->fieldArgs[$fieldID] = func_get_args();
			$this->fieldTypes[$fieldID] = $type;
		}

		function fieldIDFromName($name) {
			for($i=0; $i<$this->fieldCount; $i++)
				if ($this->fields[$i] == $name) return $i;
			return -1;
		}


		function addForm($displayFieldsList) {
			$displayFields = split(",", $displayFieldsList);
			echo "<H2>Add</H2>";

			$action = $this->urlPrefix;
			$action .= (!empty($this->urlVars)) ? "?" . $this->urlVars . "&$this->actionVariable=addSubmit" : "/?$this->actionVariable=addSubmit";

			echo "<FORM METHOD=\"post\" ACTION=\"$action\">\n";
			echo "<INPUT TYPE=\"hidden\" NAME=\"start\" VALUE=\"$this->sqlStart\">\n";
			echo '<TABLE WIDTH="100%" BORDER=0 ALIGN="center">';
			for($i=0; $i<$this->fieldCount; $i++) {
				$fieldName = $this->fields[$i];
				if (in_array($fieldName, $displayFields)) {
					echo "<TR>";
						echo "<TD VALIGN=\"top\">" . ucwords($fieldName) ."</TD>";
						echo "<TD VALIGN=\"top\">";
							$this->formInput($fieldName);
						echo "</TD>";
					echo "</TR>\n";
				}
			}
			?>
			<TR><TD COLSPAN=<?= $this->fieldCount ?> ALIGN="center"><INPUT TYPE="submit" VALUE="Add"></TD></TR>
			</TABLE>
			</FORM>
			<?
		}


		function addFormSubmit($displayFieldsList) {
			$displayFields = split(",", $displayFieldsList);
			$c = 0;

			$fieldCount = count($displayFields);

			if ($this->version < 410) global $HTTP_POST_VARS;
			$post_vars = ($this->version < 410) ? $HTTP_POST_VARS : $_POST;

			$qryb = ""; $qrya = "";
			foreach($post_vars as $var => $val) {
				if (in_array($var, $displayFields)) {
					$c++;
					$qrya .= "`$var`";
					if ($c < $fieldCount) $qrya .= ", ";

					/* make any needed translations on the field */
					$val = $this->translate_value($var, $val);

					$qryb .= "'" . mysql_escape_string($val) . "'";
					if ($c < $fieldCount) $qryb .= ", ";
				}
			}
			$qry = "INSERT INTO `$this->table` (" . $qrya . ') VALUES(' . $qryb . ')';
			$sql = mysql_query($qry, $this->mysqlConnection);
		}

		function editForm($displayFieldsList, $id) {
			$displayFields = split(",", $displayFieldsList);
			$this->id = $id;

			$action = $this->urlPrefix;
			$action .= (!empty($this->urlVars)) ? "?" . $this->urlVars . "&" : "/?";
			$action .= "$this->actionVariable=editSubmit&$this->uniqueField=$id";

			?>
			<H2>Edit</H2>
			<FORM METHOD="post" ACTION="<?= $action ?>">
			<INPUT TYPE="hidden" NAME="start" VALUE="<?= $this->sqlStart ?>">
			<TABLE WIDTH="100%" BORDER=0 ALIGN="center">
			<?
			$sql = mysql_query("SELECT * FROM `$this->table` WHERE $this->uniqueField = '$id'", $this->mysqlConnection);
			if (!$sql) return -1;
			$obj = mysql_fetch_object($sql);

			for($i=0; $i<$this->fieldCount; $i++) {
				$fieldName = $this->fields[$i];
				$this->value = $obj->$fieldName;
				if (in_array($fieldName, $displayFields)) {
					echo "<TR>";
						echo "<TD VALIGN=\"top\">" . ucwords($fieldName) ."</TD>";
						echo "<TD VALIGN=\"top\">";
							$this->formInput($fieldName, $obj->$fieldName);
						echo "</TD>";
					echo "</TR>\n";
				}
			}
			?>
			<TR><TD COLSPAN=<?= $this->fieldCount ?> ALIGN="center"><INPUT TYPE="submit" VALUE="Update"></TD></TR>
			</TABLE>
			</FORM>
			<?
		}

		function editFormSubmit($displayFieldsList, $id) {
			$displayFields = split(",", $displayFieldsList);
			$qry = "UPDATE `$this->table` SET ";
			$c = count($displayFields);

			if ($this->version < 410) global $HTTP_POST_VARS;
			$post_vars = ($this->version <= 406) ? $HTTP_POST_VARS : $_POST;

			for($i=0; $i<$c; $i++) {
				$fieldName = $displayFields[$i];
				$fieldValue = $post_vars[$fieldName];

				$fieldValue = $this->translate_value($fieldName, $fieldValue);

				$qry .= "`$fieldName`='" . mysql_escape_string(stripslashes($fieldValue)) . "'";
				if ($i < $c - 1) $qry .= ", ";
			}
			$qry .= " WHERE $this->uniqueField = '$id'";

			$sql = mysql_query($qry, $this->mysqlConnection);
			if (!$sql) echo mysql_error($this->mysqlConnection);
		}

		function deleteForm($id) {
			$sql = mysql_query("DELETE FROM `$this->table` WHERE $this->uniqueField='$id'", $this->mysqlConnection);
		}

		function customFunction($funcName, $value) {
			/* $func_args[2] is the first dynamic argument */
			$func_args = $this->fieldFuncArgs[$funcName];
			switch ($funcName) {
				case "boolean":
					if (!empty($func_args[3]))
						list($yes, $no) = split("/", $func_args[3]);
					else {
						$yes = "Yes";
						$no = "No";
					}
					return ($value) ? $yes : $no;
					break;

				case "date":
					$format = (empty($func_args[2])) ? "M d Y" : $func_args[2];
					return date($value, $format);
					break;

				case "timestamp":
					$format = (empty($func_args[3])) ? "M d Y" : $func_args[3];
					return formatTime($value, $format);
					break;

				default:
					return $value;
					break;
			}
		}


		function displayForm($displayFieldsList, $start = -1, $offset = -1) {
			if ($start == -1) $start = $this->sqlStart;
			if ($offset == -1) $offset = $this->sqlOffset;
			$displayFields = split(",", $displayFieldsList);
			$modes = split(",", $this->mode);
			$rowColors = split(",", $this->rowColors);
			$rowColorNum = 0;
			$rowColorCount = count($rowColors);

			echo '<TABLE WIDTH="100%" BORDER=0 ALIGN="center" CELLSPACING=0 CELLPADDING=' . $this->cellpadding . '>';
			echo "<TR><TD COLSPAN=$c>\n";
			if (in_array("add", $modes)) {
				$action = $this->urlPrefix;
				$action .= (!empty($this->urlVars)) ? "?" . $this->urlVars . "&$this->actionVariable=add" : "/?$this->actionVariable=add";
				echo "<LI><A HREF=\"$action\">Add</A></LI>\n";
			}
			?>
			</TD></TR>
			<TR><TD COLSPAN=<?= $this->fieldCount + 1 ?>>&nbsp;</TD></TR>
			<TR>
			<?
			for($i=0; $i<$this->fieldCount; $i++) {
				$fieldName = $this->fields[$i];
				if (in_array($fieldName, $displayFields)) echo "<TD><B>" . ucwords($fieldName) . "</B></TD>";
			}
			echo "</TR>";

			$sql = mysql_query("SELECT COUNT(*) FROM `$this->table` $this->sqlWhere", $this->mysqlConnection);
			@list($totalRows) = mysql_fetch_row($sql);

			$sql = mysql_query("SELECT * FROM `$this->table` $this->sqlOrderBy LIMIT $start,$this->sqlOffset", $this->mysqlConnection);


			if ($sql && mysql_num_rows($sql)) {
				$pstart = $start - $this->sqlOffset;
				$nstart = $start + $this->sqlOffset;

				$action = $this->urlPrefix;
				$action .= (!empty($this->urlVars)) ? "?" . $this->urlVars . "&" : "/?";

				if ($pstart >= 0 || $nstart <= $totalRows) {
					echo "<TR><TD COLSPAN=" . ($this->fieldCount + 1) . " ALIGN=\"right\" NOWRAP>";
					if ($pstart >= 0) 
						echo "<A HREF=\"" . $action . "start=$pstart\"><< Previous $this->sqlOffset</A>";
					else
						echo "<< Previous $this->sqlOffset";
					echo " &nbsp; ";

					if ($nstart <= $totalRows)
						echo "<A HREF=\"" . $action . "start=$nstart\">Next $this->sqlOffset >></A>";
					else
						echo "Next $this->sqlOffset >>";
					echo "</TD></TR>";
				}

				echo "<TR><TD COLSPAN=" . ($this->fieldCount + 1) . ">&nbsp;</TD></TR>";

				$tmp = $this->uniqueField;
				while ($obj = mysql_fetch_object($sql)) {
					echo "<TR BGCOLOR=\"" . $rowColors[$rowColorNum] . "\">";
					$rowColorNum++;
					if ($rowColorNum == $rowColorCount) $rowColorNum = 0;

					for($i=0; $i<$this->fieldCount; $i++) {
						$fieldName = $this->fields[$i];
						if (in_array($fieldName, $displayFields)) {
							echo "<TD ALIGN=\"left\" VALIGN=\"top\">";
							$fieldValue = $obj->$fieldName;
							$fieldFunc = $this->fieldFuncs[$i];
							if (substr($fieldFunc, 0, 9) == "built-in:") {
								$fieldFunc = substr($fieldFunc, 9);
								$fieldValue = $this->customFunction($fieldFunc, $fieldValue);
							} else {
								if (function_exists($fieldFunc)) {
									$fieldValue = $fieldFunc($fieldValue);
								}
							}
							echo htmlspecialchars($fieldValue);
							echo "</TD>";
						}
					}
					$objID = $obj->$tmp;
					echo "<TD ALIGN=\"center\" NOWRAP>";
						if (in_array("edit", $modes))
							echo "<A HREF=\"" . $action . "$this->actionVariable=edit&$this->uniqueField=$objID&start=$start\">edit</A> &nbsp; ";
						if (in_array("delete", $modes))
							echo "<A HREF=\"" . $action . "$this->actionVariable=delete&$this->uniqueField=$objID&start=$start\">delete</A>";
					echo "</TD>";
					echo "</TR>";
				}
			} else {
				echo "<TR><TD COLSPAN=" . ($this->fieldCount + 1) . " ALIGN=\"center\"><I>No records found</I></TD></TR>";
			}

			echo "</TABLE>";
		}

	}
?>
