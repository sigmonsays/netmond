CC=gcc
LIBS=-lmysqlclient -L/usr/local/mysql/lib/mysql -lm
CFLAGS=-I/usr/local/mysql/include -Iinclude/ -Wall

BINARY=netmond
OBJECTS=config.o commands.o check-ips.o netcheck.o sqlSupport.o notifications.o net-io.o smtp-test.o pop3-test.o imap-test.o rad-test.o http-test.o logging.o 

all: ${OBJECTS}
	gcc ${CFLAGS} ${LIBS} -o ${BINARY} ${OBJECTS} ${BINARY}.c

clean:
	rm -f ${BINARY} *.o
	echo -n > valgrind.log

install: 
	install -m 0755 -o root -g root ${BINARY} /usr/local/bin/

