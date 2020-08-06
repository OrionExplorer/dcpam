# libpq-dev
# unixodbc-dev
# libmysqlclient-dev
# oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm
# oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm
CC=gcc


#CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -Os -O3 -O2 -O1
CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -g3 -O0
ORACLE_DEP= -I/usr/include/oracle/19.6/client64/ -L/usr/lib/oracle/19.6/client64/lib/
LIBS=-lm -lpthread -lpq -lodbc -lmariadbclient $(ORACLE_DEP) -ldl -lclntsh


all: dcpam-etl dcpam-wds dcpam-exec


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
	$(CC) -c src/db/oracle.c $(CFLAGS) $(ORACLE_DEP)

sqlite.o: src/db/sqlite.c
	$(CC) -c src/db/sqlite.c $(CFLAGS)

dcpam-etl.o: src/DCPAM_ETL/dcpam-etl.c
	$(CC) -c src/DCPAM_ETL/dcpam-etl.c $(CFLAGS) $(ORACLE_DEP)

dcpam-wds.o: src/DCPAM_WDS/dcpam-wds.c
	$(CC) -c src/DCPAM_WDS/dcpam-wds.c $(CFLAGS) $(ORACLE_DEP)

dcpam-exec.o: src/DCPAM_EXEC/dcpam-exec.c
	$(CC) -c src/DCPAM_EXEC/dcpam-exec.c $(CFLAGS)

log.o: src/utils/log.c
	$(CC) -c src/utils/log.c $(CFLAGS)

worker.o: src/core/db/worker.c
	$(CC) -c src/core/db/worker.c $(CFLAGS) $(ORACLE_DEP)

system.o: src/core/db/system.c
	$(CC) -c src/core/db/system.c $(CFLAGS) $(ORACLE_DEP)

extract.o: src/core/db/etl/extract.c
	$(CC) -c src/core/db/etl/extract.c $(CFLAGS) $(ORACLE_DEP)

stage.o: src/core/db/etl/stage.c
	$(CC) -c src/core/db/etl/stage.c $(CFLAGS) $(ORACLE_DEP)

transform.o: src/core/db/etl/transform.c
	$(CC) -c src/core/db/etl/transform.c $(CFLAGS) $(ORACLE_DEP)

load.o: src/core/db/etl/load.c
	$(CC) -c src/core/db/etl/load.c $(CFLAGS) $(ORACLE_DEP)

client.o: src/core/network/client.c
	$(CC) -c src/core/network/client.c $(CFLAGS)

socket_io.o: src/core/network/socket_io.c
	$(CC) -c src/core/network/socket_io.c $(CFLAGS)

time.o: src/utils/time.c
	$(CC) -c src/utils/time.c $(CFLAGS)

filesystem.o: src/utils/filesystem.c
	$(CC) -c src/utils/filesystem.c $(CFLAGS)

cJSON.o: src/third-party/cJSON.c
	$(CC) -c src/third-party/cJSON.c $(CFLAGS)

sqlite3.o: src/third-party/sqlite3.c
	$(CC) -c src/third-party/sqlite3.c $(CFLAGS)

misc.o: src/utils/misc.c
	$(CC) -c src/utils/misc.c $(CFLAGS)

memory.o: src/utils/memory.c
	$(CC) -c src/utils/memory.c $(CFLAGS)

strings.o: src/utils/strings.c
	$(CC) -c src/utils/strings.c $(CFLAGS)

dcpam-exec: dcpam-exec.o socket_io.o log.o time.o filesystem.o memory.o
	$(CC) socket_io.o dcpam-exec.o log.o time.o filesystem.o memory.o -o dcpam-exec

dcpam-etl: dcpam-etl.o mysql.o mariadb.o odbc.o postgresql.o log.o time.o filesystem.o cJSON.o sqlite3.o memory.o db.o worker.o system.o extract.o stage.o transform.o load.o strings.o oracle.o sqlite.o client.o
	$(CC) mysql.o mariadb.o odbc.o postgresql.o dcpam-etl.o log.o time.o filesystem.o cJSON.o sqlite3.o memory.o db.o worker.o system.o extract.o stage.o transform.o load.o strings.o oracle.o sqlite.o client.o -o dcpam-etl $(LIBS)

dcpam-wds: dcpam-wds.o mysql.o mariadb.o odbc.o postgresql.o log.o time.o filesystem.o cJSON.o sqlite3.o memory.o db.o worker.o system.o extract.o stage.o transform.o load.o strings.o oracle.o sqlite.o client.o
	$(CC) mysql.o mariadb.o odbc.o postgresql.o dcpam-wds.o log.o time.o filesystem.o cJSON.o sqlite3.o memory.o db.o worker.o system.o extract.o stage.o transform.o load.o strings.o oracle.o sqlite.o client.o -o dcpam-wds $(LIBS)
