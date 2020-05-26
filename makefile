# libpq-dev
# unixodbc-dev
# libmysqlclient-dev
CC=gcc 

CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -g3
#CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -Os -O3 -O2 -O1
LIBS=-lm -lpthread -lpq -lmysqlclient -lodbc


all: dcpam


db.o: src/db/db.c
	gcc -c src/db/db.c $(CFLAGS)

postgresql.o: src/db/postgresql.c
	gcc -c src/db/postgresql.c $(CFLAGS)

mysql.o: src/db/mysql.c
	gcc -c src/db/mysql.c $(CFLAGS)

mssql.o: src/db/mssql.c
	gcc -c src/db/mssql.c $(CFLAGS)

dcpam.o: src/dcpam.c
	gcc -c src/dcpam.c $(CFLAGS)

log.o: src/utils/log.c
	gcc -c src/utils/log.c $(CFLAGS)

worker.o: src/core/db/worker.c
	gcc -c src/core/db/worker.c $(CFLAGS)

system.o: src/core/db/system.c
	gcc -c src/core/db/system.c $(CFLAGS)

extract.o: src/core/db/cdc/extract.c
	gcc -c src/core/db/cdc/extract.c $(CFLAGS)

transform.o: src/core/db/cdc/transform.c
	gcc -c src/core/db/cdc/transform.c $(CFLAGS)

load.o: src/core/db/cdc/load.c
	gcc -c src/core/db/cdc/load.c $(CFLAGS)

time.o: src/utils/time.c
	gcc -c src/utils/time.c $(CFLAGS)

filesystem.o: src/utils/filesystem.c
	gcc -c src/utils/filesystem.c $(CFLAGS)

cJSON.o: src/third-party/cJSON.c
	gcc -c src/third-party/cJSON.c $(CFLAGS)

misc.o: src/utils/misc.c
	gcc -c src/utils/misc.c $(CFLAGS)

memory.o: src/utils/memory.c
	gcc -c src/utils/memory.c $(CFLAGS)

strings.o: src/utils/strings.c
	gcc -c src/utils/strings.c $(CFLAGS)

dcpam: dcpam.o mysql.o mssql.o postgresql.o log.o time.o filesystem.o cJSON.o memory.o db.o worker.o system.o extract.o transform.o load.o strings.o
	gcc mysql.o mssql.o postgresql.o dcpam.o log.o time.o filesystem.o cJSON.o memory.o db.o worker.o system.o extract.o transform.o load.o strings.o -o dcpam $(LIBS)
	rm *.o