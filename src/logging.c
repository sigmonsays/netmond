#include "common.h"
#include "logging.h"

extern int debug;
extern int doFork;
extern int logging;

int debug_log(char *msg) {
	int f;
        f = open(DEBUG_LOG, O_WRONLY|O_APPEND|O_CREAT, S_IRWXU);
	if (f) {
		write(f, msg, strlen(msg));
		close(f);
	}
	return 0;
}

int _log(char *msg, int is_debug) {
        int f;
	char tm[255];
	time_t now;
	
	time(&now);
	sprintf(tm, "%d ", (int)now);

	if (is_debug && debug == 0) return 0;

        f = open(logfile, O_WRONLY|O_APPEND|O_CREAT, S_IRWXU);

	if (debug) printf("%s%s", tm, msg);

        if (f && logging) {

		if (msg) { 
			write(f, tm, strlen(tm));
			write(f, msg, strlen(msg));
		}
		close(f);
		return 0;
        }
	return 0;
}
