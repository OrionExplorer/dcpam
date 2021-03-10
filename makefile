CC=clang

#CFLAGS=-std=c11 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -Os -O3 -O2 -O1
CFLAGS=-std=c11 -D_XOPEN_SOURCE=600 -fexpensive-optimizations -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -pedantic-errors -pedantic -w -Wfatal-errors -Wextra -Wall -g3 -O0
ORACLE_DEP=-I/usr/include/oracle/19.6/client64/ -L/usr/lib/oracle/19.6/client64/lib/
MONGODB_DEP=-I/usr/include/libmongoc-1.0/ -I/usr/include/libbson-1.0/
LIBS=-lm -lpthread -lpq -lodbc -lmariadbclient -lldap -llber $(ORACLE_DEP) -ldl -lclntsh -lssl -lcrypto $(MONGODB_DEP) -lmongoc-1.0 -lbson-1.0


all: dcpam-etl dcpam-wds dcpam-rdp dcpam-lcs

db.o: src/db/db.c
	$(CC) -c src/db/db.c $(CFLAGS)

postgresql.o: src/db/postgresql.c
	$(CC) -c src/db/postgresql.c $(CFLAGS)

mongodb.o: src/db/mongodb.c
	$(CC) -c src/db/mongodb.c $(CFLAGS) $(MONGODB_DEP)

ldap.o: src/db/ldap.c
	$(CC) -c src/db/ldap.c $(CFLAGS)

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
	$(CC) -c src/DCPAM_ETL/dcpam-etl.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

dcpam-wds.o: src/DCPAM_WDS/dcpam-wds.c
	$(CC) -c src/DCPAM_WDS/dcpam-wds.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

dcpam-rdp.o: src/DCPAM_RDP/dcpam-rdp.c
	$(CC) -c src/DCPAM_RDP/dcpam-rdp.c $(CFLAGS)

dcpam-lcs.o: src/DCPAM_LCS/dcpam-lcs.c
	$(CC) -c src/DCPAM_LCS/dcpam-lcs.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

lcs_worker.o: src/DCPAM_LCS/lcs_worker.c
	$(CC) -c src/DCPAM_LCS/lcs_worker.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

wds_node.o: src/DCPAM_WDS/wds_node.c
	$(CC) -c src/DCPAM_WDS/wds_node.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

sql_parser.o: src/DCPAM_WDS/sql_parser.c
	$(CC) -c src/DCPAM_WDS/sql_parser.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

log.o: src/utils/log.c
	$(CC) -c src/utils/log.c $(CFLAGS)

worker.o: src/core/db/worker.c
	$(CC) -c src/core/db/worker.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

system.o: src/core/db/system.c
	$(CC) -c src/core/db/system.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

extract.o: src/core/db/etl/extract.c
	$(CC) -c src/core/db/etl/extract.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

stage.o: src/core/db/etl/stage.c
	$(CC) -c src/core/db/etl/stage.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

transform.o: src/core/db/etl/transform.c
	$(CC) -c src/core/db/etl/transform.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

load.o: src/core/db/etl/load.c
	$(CC) -c src/core/db/etl/load.c $(CFLAGS) $(ORACLE_DEP) $(MONGODB_DEP)

client.o: src/core/network/client.c
	$(CC) -c src/core/network/client.c $(CFLAGS)

socket_io.o: src/core/network/socket_io.c
	$(CC) -c src/core/network/socket_io.c $(CFLAGS)

http.o: src/core/network/http.c
	$(CC) -c src/core/network/http.c $(CFLAGS)

csv.o: src/file/csv.c
	$(CC) -c src/file/csv.c $(CFLAGS)

json.o: src/file/json.c
	$(CC) -c src/file/json.c $(CFLAGS)

preload.o: src/core/file/preload.c
	$(CC) -c src/core/file/preload.c $(CFLAGS) $(MONGODB_DEP)

cache.o: src/core/cache.c
	$(CC) -c src/core/cache.c $(CFLAGS) $(MONGODB_DEP)

lcs_report.o: src/core/lcs_report.c
	$(CC) -c src/core/lcs_report.c $(CFLAGS)

component.o: src/core/component.c
	$(CC) -c src/core/component.c $(CFLAGS)

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

regex.o: src/utils/regex.c
	$(CC) -c src/utils/regex.c $(CFLAGS)

strings.o: src/utils/strings.c
	$(CC) -c src/utils/strings.c $(CFLAGS)

dcpam-rdp: dcpam-rdp.o socket_io.o log.o time.o strings.o client.o http.o filesystem.o memory.o cJSON.o lcs_report.o
	@ mkdir bin -p
	$(CC) socket_io.o dcpam-rdp.o log.o time.o strings.o client.o http.o filesystem.o memory.o cJSON.o lcs_report.o -o bin/dcpam-rdp -lpthread -lssl -lcrypto

dcpam-etl: dcpam-etl.o mysql.o mariadb.o odbc.o postgresql.o mongodb.o ldap.o log.o time.o filesystem.o cJSON.o sqlite3.o memory.o regex.o http.o preload.o csv.o json.o db.o worker.o system.o extract.o stage.o transform.o load.o strings.o oracle.o sqlite.o client.o lcs_report.o socket_io.o
	@ mkdir bin -p
	$(CC) mysql.o mariadb.o odbc.o postgresql.o mongodb.o dcpam-etl.o log.o time.o filesystem.o cJSON.o sqlite3.o memory.o regex.o http.o preload.o csv.o json.o db.o worker.o system.o extract.o stage.o transform.o load.o ldap.o strings.o oracle.o sqlite.o client.o lcs_report.o socket_io.o -o bin/dcpam-etl $(LIBS)

dcpam-wds: dcpam-wds.o socket_io.o cache.o mysql.o mariadb.o odbc.o postgresql.o mongodb.o ldap.o log.o time.o http.o filesystem.o cJSON.o sqlite3.o memory.o db.o system.o strings.o oracle.o sqlite.o client.o lcs_report.o wds_node.o sql_parser.o
	@ mkdir bin -p
	$(CC) cache.o mysql.o socket_io.o mariadb.o odbc.o postgresql.o mongodb.o ldap.o dcpam-wds.o log.o time.o http.o filesystem.o cJSON.o sqlite3.o memory.o db.o system.o strings.o oracle.o sqlite.o client.o lcs_report.o wds_node.o sql_parser.o -o bin/dcpam-wds $(LIBS)

dcpam-lcs: dcpam-lcs.o socket_io.o log.o time.o memory.o filesystem.o http.o client.o strings.o cJSON.o component.o lcs_worker.o
	@ mkdir bin -p
	$(CC) dcpam-lcs.o socket_io.o log.o time.o memory.o filesystem.o http.o client.o strings.o cJSON.o component.o lcs_worker.o -o bin/dcpam-lcs $(LIBS)
	@ cp -R ./conf ./bin/
	@ rm -rf ./bin/whitespaces
	@ mkdir ./bin/whitespaces
	@ cp -R ./src/DCPAM_RDP/whitespaces ./bin/
