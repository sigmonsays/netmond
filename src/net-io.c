#include "common.h"
#include "net-io.h"
#include "config.h"

/* send and receive shit in one call 
	returns response on success, NULL on error
	times out after 4 seconds
*/
char *net_request(int sock, char *req) {
	int r, s;
	char *buffer = NULL;

	/* wait till we can write 
		we can usually always write, it's up to the tcp/ip stack to deliver the message, but it can't hurt
	*/
	if (wait_io(0, sock, 4, 0) == -1) {
		return NULL;
	}

	r = send(sock, req, strlen(req), 0);

	/* wait for response -- 4 sec */
	if (wait_io(1, sock, 4, 0) == -1) {
		close(sock);
		return NULL;
	}

	buffer = (void *) malloc(1024);

	s = read(sock, buffer, 1024);

	if (s == -1) {
		free(buffer);
		return NULL;
	}
	if (s == 0) {
		free(buffer);
		return NULL;
	}

	if (s > 0) {
		*(buffer + s) = '\0';
		return buffer;
	}

	return NULL;
}

/* function that waits for available read/write, returns -1 on timeout/error, 0 on i/o available
	mode 0 = write
	mode 1 = read
*/
int wait_io(int mode, int sockfd, int timeout_sec, int timeout_usec) {
	int r;
	fd_set fds;
	struct timeval timeout;

	if (mode != 0 && mode != 1) return -1;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = timeout_usec;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	if (mode == 0) {
		r = select(sockfd + 1, NULL, &fds, NULL, &timeout);

	} else if (mode == 1) {
		r = select(sockfd + 1, &fds, NULL, NULL, &timeout);
	}

	if (r == -1) return -1;

	if (FD_ISSET(sockfd, &fds)) {
		return 0;
	} else {
		return -1;
	}
}
