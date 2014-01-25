#include "mysql/mysql.h"
#include <unistd.h>

#ifndef HAVE_CONFIG_H
#include "config.h"
#endif

#define HAVE_SQL_SUPPORT_H

struct host_ent {
	int id;
	char *hostname, *ports, *name;
	int group;
};

struct sql_data {
	void *data;
};

MYSQL *init_sql();
void close_sql(MYSQL *mysqlConnection);
int enumerate_hosts(MYSQL *mysqlConnection, struct host_ent *host, struct sql_data *sql_res);
int backlog(char *host, int port, int state);
