
CREATE TABLE `contactHosts` (
  `id` int(11) NOT NULL auto_increment,
  `sid` int(11) NOT NULL default '0',
  `contactids` char(255) NOT NULL default '',
  KEY `id` (`id`)
) TYPE=MyISAM;

CREATE TABLE `groups` (
  `id` bigint(20) NOT NULL auto_increment,
  `name` char(128) NOT NULL default '',
  KEY `id` (`id`)
) TYPE=MyISAM;

CREATE TABLE `notifications` (
  `id` bigint(20) NOT NULL auto_increment,
  `fullName` char(128) NOT NULL default '',
  `email` char(255) NOT NULL default '',
  `pager` char(11) NOT NULL default '',
  `send_email` int(11) NOT NULL default '0',
  `send_page` int(11) NOT NULL default '0',
  KEY `id` (`id`)
) TYPE=MyISAM;

CREATE TABLE `servers` (
  `id` bigint(20) NOT NULL auto_increment,
  `host` varchar(255) NOT NULL default '',
  `group` int(11) NOT NULL default '0',
  `ports` varchar(255) NOT NULL default '',
  `name` varchar(32) NOT NULL default '',
  KEY `id` (`id`)
) TYPE=MyISAM;

CREATE TABLE `status` (
  `serverid` int(11) NOT NULL default '0',
  `port` int(11) NOT NULL default '0',
  `status` int(11) NOT NULL default '0'
) TYPE=MyISAM;

CREATE TABLE backlog (
  id bigint(20) NOT NULL auto_increment,
  host char(16) NOT NULL default '',
  port int(11) NOT NULL default '0',
  when timestamp(14) NOT NULL,
  state enum('UP','DOWN') NOT NULL default 'UP',
  KEY id (id)
) TYPE=MyISAM;
