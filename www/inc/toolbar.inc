<?
	require("class/toolbar.inc");

	$toolbar = new Toolbar("Netmond");

	$tid = $toolbar->addheading("Main");
	$toolbar->addLink($tid, "Status", $urlPrefix);

	$tid = $toolbar->addHeading("Groups");
	$toolbar->addLink($tid, "Configure", "$PHP_SELF?cat=groups");

	$tid = $toolbar->addHeading("Hosts");
	$toolbar->addLink($tid, "Configure", "$PHP_SELF?cat=hosts");

	$tid = $toolbar->addHeading("Notifications");
	$toolbar->addLink($tid, "Configure", "$PHP_SELF?cat=notifications");


	$toolbar->display();
?>
