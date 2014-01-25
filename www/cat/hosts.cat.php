<?
	if (!defined("VALID")) die;
	if (!defined("ADMIN")) die($str_not_allowed);

	require("class/formInput.class.inc");
	require("class/sqlForm.class.inc");

	$form = new sqlForm($mysqlConnection, "servers", $dbDatabase);

	$form->debug = 1;

	$form->urlPrefix = "$urlPrefix";
	$form->urlVars = "cat=$cat";
	$form->uniqueField = "id";

	$form->sqlOrderBy = "ORDER BY `group` ASC, id ASC";

	$form->setFieldType("ports", "text", 30);

	$groups = array();
	$sql = mysql_query("SELECT * FROM groups ORDER BY name ASC", $mysqlConnection);
	while ($group= mysql_fetch_object($sql)) {
		$groups[$group->id] = $group->name;
	}

	$form->setFieldType("group", "select", $groups);

	$addFields = "name,host,ports,group";

	switch ($_GET['action']) {

		case "add":
			$form->addForm($addFields);
			break;

		case "addSubmit":
			$form->addFormSubmit($addFields);
			redir("$urlPrefix/?cat=$cat");
			break;

		case "edit":
			$form->editForm($addFields, $_GET['id']);
			break;

		case "editSubmit":
			$form->editFormSubmit($addFields, $_GET['id']);
			redir("$urlPrefix/?cat=$cat");
			break;

		case "delete":
			$form->deleteForm($_GET['id']);
			redir("$urlPrefix/?cat=$cat");
			break;

		default:
			$form->displayForm("name,ports", intval($start), intval($offset));
			break;
	}
?>
