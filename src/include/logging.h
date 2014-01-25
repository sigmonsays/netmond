#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

extern int logging;
extern char *logfile;

int _log(char *msg, int is_debug);
int debug_log(char *msg);

