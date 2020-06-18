# DCPAM [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
###### _Data Construct-Populate-Access-Manage_ 
#### Data warehouse engine
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![Linux ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png)

* Data warehouse or data mart engine.
* DCPAM helps to create central repositories of integrated data from one or disparate sources([1]).
* Multiplatform (Linux/Windows).

### Architecture
#### Overview
* DCPAM is ETL - based solution([2]).
* Each Change Data Capture process operates independently within separated thread.
* Multiple instances of DCPAM with different configuration can run on single server.
* Most database support is provided with native libraries (see _Data sources_ and _Linux Dependencies_ in this document).

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)

####  ETL and Change Data Capture
Chance Data Capture([3]) solutions depends on the data sources (currently it's database only):

| CDC solution                            | Source        |
|-----------------------------------------|:-------------:| 
| SQL query for timestamps (eg. MIN, MAX) | Database      |
| SQL query for diffs (eg. IN, NOT IN)    | Database      |


> **Notice**: Only Extract and Load processes are available. It is yet to be decided how to handle Transform process.

### Data sources
DCPAM is still work in progress, with following data sources:
|  ID  | Data source     | Type            | Support | Status      |
|:----:|:--------------  |:---------------:|:-------:|:-----------:|
| 1    | PostgreSQL      | Database        | Native  | Available   |
| 2    | MySQL 8         | Database        | Native  | Available   |
| 3    | MariaDB/MySQL 5 | Database        | Native  | Available   |
| 4    | SQL Server      | Database        | ODBC    | Available   |
| 5    | Oracle Database | Database        | Native  | Available   |


### Configuration
File `config.json` is DCPAM foundation. It defines:
* Data sources
* Extract, Transform and Load process for each data source
* DCPAM database, tabels and views (see _app.DATA_), where integrated data is stored


#### Compilation (Linux)
```
> make
```
- run
```
> ./dcpam
```
- run with Valgrind (with suppression of Oracle OCI errors) and default `config.json`.
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_oci.supp ./dcpam
```
- run with Valgrind and `config_mysql.json` (where MySQL is DCPAM main database)
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no ./dcpam config_mysql.json
```

##### Linux Dependencies
- libpq-dev (PostgreSQL)
- libmysqlclient-dev (MySQL)
- libmariadbclient-dev (MariaDB)
- unixodbc-dev (ODBC)
- oracle-instantclient19.6-basic-19.6.0.0.0-1.x86_64.rpm (Oracle Instant Client)
- oracle-instantclient19.6-devel-19.6.0.0.0-1.x86_64.rpm (Oracle Instant Client SDK)

##### Windows Dependencies
- MariaDB Connector C
- MySQL Connector 8.0
- PostgreSQL
- Oracle Database


This software uses [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.

[1]: https://en.wikipedia.org/wiki/Data_warehouse
[2]: https://en.wikipedia.org/wiki/Extract,_transform,_load
[3]: https://en.wikipedia.org/wiki/Change_data_capture
