#include "common.h"
#include "http-test.h"
#include "net-io.h"
#include "config.h"
#include "netmond.h"

extern struct mconfig *main_config;

/*
	function to login and send mail via smtp
	returns 0 on sucess, 1 on closed, -1 on error
*/

int check_http(char *host) {
	int sockfd;
	struct sockaddr_in dest_addr;
#ifdef USE_GETHOSTBYNAME
        struct hostent *hp;
#endif
	char buf[] = "GET /robots.txt HTTP/1.0\r\n\r\n";
	char *tmp;

	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

#ifdef USE_GETHOSTBYNAME
        if ((hp = gethostbyname(host)) == NULL) {
                return -1;
        }
#endif

	sockfd = socket(AF_INET, SOCK_STREAM, 0);


	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(80);

#ifdef USE_GETHOSTBYNAME
        dest_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
#else
        dest_addr.sin_addr.s_addr = inet_addr(host);
#endif

	/* this will usually return -1 on a non-blocking socket */
	connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

	if (wait_io(0, sockfd, 4, 0) == -1) {
		shutdown(sockfd, 2);
		close(sockfd);
	}

	/* send get */
	if (!(tmp = net_request(sockfd, buf))) {
		shutdown(sockfd, 2);
		close(sockfd);
		return 1;
	}

	free(tmp);
	shutdown(sockfd, 2);
	close(sockfd);

	if (strlen(tmp) == 0) {
		return 1;
	} else {
		return 0;
	}
}
