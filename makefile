# libpq-dev
# unixodbc-dev
# libmysqlclient-dev
# oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm
# oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm
CC=clang 


#CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -Os -O3 -O2 -O1
CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -g3 -O0
LIBS=-lm -lpthread -lpq -lodbc -lmariadbclient -L/usr/lib/oracle/19.6/client64/lib/ -lclntsh
ORACLE_INC= -I/usr/include/oracle/19.6/client64/


all: dcpam


db.o: src/db/db.c
	$(CC) -c src/db/db.c $(CFLAGS)

postgresql.o: src/db/postgresql.c
	$(CC) -c src/db/postgresql.c $(CFLAGS)

mysql.o: src/db/mysql.c
	$(CC) -c src/db/mysql.c $(CFLAGS)

mariadb.o: src/db/mariadb.c
	$(CC) -c src/db/mariadb.c $(CFLAGS)

odbc.o: src/db/odbc.c
	$(CC) -c src/db/odbc.c $(CFLAGS)

oracle.o: src/db/oracle.c
	$(CC) -c src/db/oracle.c $(CFLAGS) $(ORACLE_INC)

dcpam.o: src/dcpam.c
	$(CC) -c src/dcpam.c $(CFLAGS) $(ORACLE_INC)

log.o: src/utils/log.c
	$(CC) -c src/utils/log.c $(CFLAGS)

worker.o: src/core/db/worker.c
	$(CC) -c src/core/db/worker.c $(CFLAGS) $(ORACLE_INC)

system.o: src/core/db/system.c
	$(CC) -c src/core/db/system.c $(CFLAGS) $(ORACLE_INC)

extract.o: src/core/db/cdc/extract.c
	$(CC) -c src/core/db/cdc/extract.c $(CFLAGS) $(ORACLE_INC)

transform.o: src/core/db/cdc/transform.c
	$(CC) -c src/core/db/cdc/transform.c $(CFLAGS) $(ORACLE_INC)

load.o: src/core/db/cdc/load.c
	$(CC) -c src/core/db/cdc/load.c $(CFLAGS) $(ORACLE_INC)

time.o: src/utils/time.c
	$(CC) -c src/utils/time.c $(CFLAGS)

filesystem.o: src/utils/filesystem.c
	$(CC) -c src/utils/filesystem.c $(CFLAGS)

cJSON.o: src/third-party/cJSON.c
	$(CC) -c src/third-party/cJSON.c $(CFLAGS)

misc.o: src/utils/misc.c
	$(CC) -c src/utils/misc.c $(CFLAGS)

memory.o: src/utils/memory.c
	$(CC) -c src/utils/memory.c $(CFLAGS)

strings.o: src/utils/strings.c
	$(CC) -c src/utils/strings.c $(CFLAGS)

dcpam: dcpam.o mysql.o mariadb.o odbc.o postgresql.o log.o time.o filesystem.o cJSON.o memory.o db.o worker.o system.o extract.o transform.o load.o strings.o oracle.o
	$(CC) mysql.o mariadb.o odbc.o postgresql.o dcpam.o log.o time.o filesystem.o cJSON.o memory.o db.o worker.o system.o extract.o transform.o load.o strings.o oracle.o -o dcpam $(LIBS)
	rm *.o