#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HAVE_CONFIG_H

struct st_server {
	char *host, *user, *pass, *email, *secret;
	int port;
};

struct st_ips {
	char *ip;
	struct st_ips *next;
};

struct mconfig {
	char *cfg_file;

	int interval;

	int database;
	char *dbHost, *dbUser, *dbPass, *dbDatabase;

	struct st_server *smtp;
	struct st_server *pop3;
	struct st_server *imap;
	struct st_server *rad;

	struct st_ips *allow_ips_head;
	struct st_ips *allow_ips;

	int notifications;

	time_t last_check;
};




struct mconfig *parse_cfg(char *config_file);
void skipline(FILE *f);
int *parse_ports(char *ports, int *max_num);
struct cfg *loop_config(struct mconfig *mf);
void free_config(struct mconfig *cfg);
void free_st_server(struct st_server *st);
struct st_server *new_st_server();
