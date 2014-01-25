#include "common.h"
#include "sqlSupport.h"
#include "logging.h"

extern struct mconfig *main_config;

/*
	initilize sql connecion -- 
	returns 1 upon success, 0 on failure
*/
MYSQL *init_sql() {

	char buffer[255];
	MYSQL *mysqlConnection = NULL;

	mysqlConnection = mysql_init(mysqlConnection);

	if (mysqlConnection == NULL) {
		_log("init_sql: mysql_init failed!\n", 0);
		return NULL;
	}
	mysqlConnection = mysql_connect(mysqlConnection, main_config->dbHost, main_config->dbUser, main_config->dbPass);

	if (mysqlConnection == NULL) {
		sprintf(buffer, "init_sql: Failed to connect to sql on %s\n", main_config->dbHost);
		_log(buffer, 0);
	} else {
		mysql_select_db(mysqlConnection, main_config->dbDatabase);
	}
	return mysqlConnection;
}

void close_sql(MYSQL *mysqlConnection) {
	if (mysqlConnection) {
		mysqlConnection->free_me = 1;
		mysql_close(mysqlConnection);
		mysqlConnection = NULL;
	}
}

int enumerate_hosts(MYSQL *mysqlConnection, struct host_ent *host, struct sql_data *sql_res) {

	MYSQL_ROW mysql_row;
	char qry[1024];
	int ql;

	if (host == NULL) {
		strcpy(qry, "SELECT * FROM servers ORDER BY id");
		ql = strlen(qry);
		if ( mysql_real_query(mysqlConnection, qry, ql) != 0) return 2;

		sql_res->data = mysql_store_result(mysqlConnection);

		return 0;
	}

	mysql_row = mysql_fetch_row(sql_res->data);

	if (!mysql_row) {
		mysql_free_result(sql_res->data);
		return 1;
	}

	host->id = atoi(mysql_row[0]);
	host->hostname = strdup(mysql_row[1]);
	host->group = atoi(mysql_row[2]);
	host->ports = strdup(mysql_row[3]);
	host->name = strdup(mysql_row[4]);
	return 0;
}

/* add record to backlog table 
	state is either 1 (up) or 2 (down)

	backlog returns 0 on success, non-zero on error

	1 mysql connection failed
	2 mysql query failed
*/
int backlog(char *host, int port, int state) {
	int ql;
	char qry[512];
	MYSQL *mysqlConnection = NULL;

	mysqlConnection = init_sql();

	if (mysqlConnection == NULL) return 1;

	ql = sprintf(qry, "INSERT INTO backlog ( host, port, state ) VALUES ( '%s', '%d', '%d' )", host, port, state);

	if ( mysql_real_query(mysqlConnection, qry, ql) != 0) {
		close_sql(mysqlConnection);
		return 2;
	}

	close_sql(mysqlConnection);
	return 0;
}
