#include "common.h"

#include "config.h"
#include "check-ips.h"

/* parse ips */
struct st_ips *parse_ips(struct mconfig *mf, char *list) {
	char *tmp;
	struct st_ips *new_ip_list;

	new_ip_list = (void *) malloc(sizeof(struct st_ips));
	if (new_ip_list == NULL) return NULL;

	mf->allow_ips = new_ip_list;

	tmp = strtok(list, " ");
	if (tmp) {
		allow_ip(mf, tmp);
		while ((tmp = strtok(NULL, " "))) {
			allow_ip(mf, tmp);
		}
	}

	return (struct st_ips *) new_ip_list;
}


/* add ip to linked list */
int allow_ip(struct mconfig *mf, char *ipaddy) {
	struct st_ips *new_ip;

	new_ip = (void *) malloc(sizeof(struct st_ips));
	if (new_ip == NULL) return NULL;

	new_ip->ip = strdup(ipaddy);

	if (mf->allow_ips_head == NULL) mf->allow_ips_head = new_ip;

	mf->allow_ips->next = new_ip;
	mf->allow_ips = mf->allow_ips->next;

	/* fucking shit -- this cause a big old problem in freeing my memory */
	mf->allow_ips->next = NULL;

	return 0;
}

/* checks if the ip is in the linked list 
	returns 1 if it is, otherwise 0
*/
int check_ip(struct mconfig *mf, char *ip) {
	int l;
	struct st_ips *allow_ips;

	allow_ips = mf->allow_ips_head;


	while (allow_ips) {
		l = strlen(allow_ips->ip);
		if (strncmp(allow_ips->ip, ip, l) == 0) return 1;
		allow_ips = allow_ips->next;
	}
	return 0;
}

