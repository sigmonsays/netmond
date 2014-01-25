#include "common.h"
#include "config.h"
#include "check-ips.h"

struct mconfig *parse_cfg(char *config_file) {
        FILE *fd;
	struct mconfig *main_config;

	char buf[1024];
	char ctype[64];
	char var[65], val[256];

        if ((fd = fopen(config_file, "r")) == NULL) {
		return NULL;
	}

	main_config = (void *) malloc(sizeof(struct mconfig));

	/* initilize everything to null */
	main_config->cfg_file = NULL;
	main_config->database = 0;

	main_config->dbUser = NULL;
	main_config->dbHost = NULL;
	main_config->dbPass = NULL;
	main_config->dbDatabase = NULL;

	main_config->smtp = NULL;
	main_config->pop3 = NULL;
	main_config->imap = NULL;
	main_config->rad = NULL;

	main_config->allow_ips = NULL;
	main_config->allow_ips_head = NULL;

	main_config->interval = 300;

	main_config->notifications = 1;

	main_config->last_check = -1;

	main_config->cfg_file = strdup(config_file);

	while (fgets(buf, 1024, fd)) {

		sscanf(buf, "[%64[a-zA-Z0-9.-]]", (char *) &ctype);

		if (strcmp(ctype, "general") == 0) {
			if (sscanf(buf, "%64s = %255[a-zA-Z0-9.-, ]", var, val) == 2) {
				if (strcmp(var, "allow") == 0) {
					if (strlen(val))
						main_config->allow_ips = parse_ips(main_config, val);
				}

				if (strcmp(var, "interval") == 0) {
					main_config->interval = atoi(val);
				}

				if (strcmp(var, "notifications") == 0) {
					main_config->notifications = atoi(val);
				}
			}

		} else if (strcmp(ctype, "database") == 0) {
			if (sscanf(buf, "%64s = %255[a-zA-Z0-9.-, ]", var, val) == 2) {
				if (strcmp(var, "host") == 0) {
					main_config->dbHost = strdup(val);

				} else if (strcmp(var, "user") == 0) {
					main_config->dbUser = strdup(val);

				} else if (strcmp(var, "pass") == 0) {
					main_config->dbPass = strdup(val);

				} else if (strcmp(var, "database") == 0) {
					main_config->dbDatabase = strdup(val);
				}
			}

			main_config->database = (main_config->dbDatabase) ? 1 : 0;

		} else if (strcmp(ctype, "smtp") == 0) {
			if (!main_config->smtp) main_config->smtp = new_st_server();

			if (sscanf(buf, "%64s = %255[a-zA-Z0-9.-, ]", var, val) == 2) {
				if (strcmp(var, "user") == 0) {
					main_config->smtp->user = strdup(val);
				} 
			}

		} else if (strcmp(ctype, "pop3") == 0) {
			if (!main_config->pop3) main_config->pop3 = new_st_server();

			if (sscanf(buf, "%64s = %255[a-zA-Z0-9.-, ]", var, val) == 2) {
				if (strcmp(var, "user") == 0) {
					main_config->pop3->user = strdup(val);

				} else if (strcmp(var, "pass") == 0) {
					main_config->pop3->pass = strdup(val);
				}
			}

		} else if (strcmp(ctype, "imap") == 0) {
			if (!main_config->imap) main_config->imap = new_st_server();

			if (sscanf(buf, "%64s = %255[a-zA-Z0-9.-, ]", var, val) == 2) {
				if (strcmp(var, "user") == 0) {
					main_config->imap->user = strdup(val);

				} else if (strcmp(var, "pass") == 0) {
					main_config->imap->pass = strdup(val);
				}
			}

		} else if (strcmp(ctype, "radius") == 0) {
			if (!main_config->rad) main_config->rad = new_st_server();

			if (sscanf(buf, "%64s = %255[a-zA-Z0-9.-, ]", var, val) == 2) {
				if (strcmp(var, "user") == 0) {
					main_config->rad->user = strdup(val);

				} else if (strcmp(var, "pass") == 0) {
					main_config->rad->pass = strdup(val);
				} else if (strcmp(var, "secret") == 0) {
					main_config->rad->secret = strdup(val);
				}
			}
		}
	}

	fclose(fd);
	return main_config;
}


void skipline(FILE *f) {
  int ch;
  do {
    ch = getc(f);
  } while ( ch != '\n' && ch != EOF );
}


/* return an array containign ports parsed from a csv string 

	pass max_num as a max of ports you want returned, after the
	function returns, max_num is changed containing the actual
	about of ports parsed.
*/
int *parse_ports(char *ports, int *max_num) {
	char *t;
	int *rports;
	int i = 0, m;
	char *buf = strdup(ports);
	m = *max_num;

	t = strtok(buf, ",");
	if (t == NULL) {
		*max_num = 0;
		return NULL;
	}

	rports = (int *) malloc(sizeof(int) * m);
	for( ; t; ) {
		rports[i++] = strtol(t, NULL, 0);
		t = strtok(NULL, ",");
	}
	*max_num = i;
	free(buf);
	return rports;
}


void free_config(struct mconfig *cfg) {
	struct st_ips *ipcfg, *ipcfg_tmp;

	free(cfg->cfg_file);

	/* free db shizat */
	if (cfg->dbHost) free(cfg->dbHost);
	if (cfg->dbUser) free(cfg->dbUser);
	if (cfg->dbPass) free(cfg->dbPass);
	if (cfg->dbDatabase) free(cfg->dbDatabase);

	free_st_server(cfg->smtp);
	free_st_server(cfg->pop3);
	free_st_server(cfg->imap);
	free_st_server(cfg->rad);

	ipcfg = cfg->allow_ips_head;

	while (ipcfg) {
		ipcfg_tmp = ipcfg;
		ipcfg = ipcfg->next;

		if (ipcfg_tmp) {
			free(ipcfg_tmp->ip);
			free(ipcfg_tmp);
		}
	}

	free(cfg->allow_ips);

	free(cfg);
	cfg = NULL;
}

void free_st_server(struct st_server *st) {
	if (st == NULL) return;

	if (st->host) free(st->host);
	if (st->user) free(st->user);
	if (st->pass) free(st->pass);
	if (st->email) free(st->email);
	if (st->secret) free(st->secret);
	free(st);
}

struct st_server *new_st_server() {
	struct st_server *st;
	st = (void *) malloc(sizeof(struct st_server));

	st->host = NULL;
	st->user = NULL;
	st->pass = NULL;
	st->email = NULL;
	st->secret = NULL;
	st->port = 0;
	return st;
}
