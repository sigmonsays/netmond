<?
	if (!defined("VALID")) die;
	if (!defined("ADMIN")) die($str_not_allowed);

	require("class/formInput.class.inc");
	require("class/sqlForm.class.inc");

	$form = new sqlForm($mysqlConnection, "notifications", $dbDatabase);

	$form->debug = 1;

	$form->urlPrefix = "$urlPrefix";
	$form->urlVars = "cat=$cat";
	$form->uniqueField = "id";

	$form->setFieldType("fullName", "text", 30);
	$form->setFieldType("email", "text", 30);
	$form->setFieldType("pager", "text", 30);
	$form->setFieldType("send_email", "boolean");
	$form->setFieldType("send_page", "boolean");

	$form->setFieldFunction("send_email", "boolean", "built-in", "Yes/No");
	$form->setFieldFunction("send_page", "boolean", "built-in", "Yes/No");

	$addFields = "fullName,email,pager,send_email,send_page";

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
			$form->displayForm("fullName,email,send_email,send_page", intval($start), intval($offset));
			break;
	}
?>
