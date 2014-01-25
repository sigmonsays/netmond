<?
	if (!defined("VALID")) die;
	if (!defined("ADMIN")) die($str_not_allowed);

	require("class/formInput.class.inc");
	require("class/sqlForm.class.inc");

	$form = new sqlForm($mysqlConnection, "groups", $dbDatabase);

	$form->debug = 1;

	$form->urlPrefix = "$urlPrefix";
	$form->urlVars = "cat=$cat";
	$form->uniqueField = "id";

	$addFields = "name";

	switch ($_GET['action']) {

		case "add":
			$form->addForm($addFields);
			break;

		case "addSubmit":
			$form->addFormSubmit($addFields);
			redir("$urlPrefix/?cat=$cat");
			break;

		case "edit":
			$form->editForm($addFields, $id);
			break;

		case "editSubmit":
			$form->editFormSubmit($addFields, $id);
			redir("$urlPrefix/?cat=$cat");
			break;

		case "delete":
			$form->deleteForm($id);
			redir("$urlPrefix/?cat=$cat");
			break;

		default:
			$form->displayForm("name", intval($start), intval($offset));
			break;
	}
?>
