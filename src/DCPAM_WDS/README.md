# DCPAM WDS [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
 
 Copyright © 2020 Marcin Kelar
###### _Data Construct-Populate-Access-Manage_ 
#### Warehouse Data Server
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png)

##### Currently under active development
* [ ] To be announced.

### Table of contents
* [Purpose of DCPAM WDS](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#purpose-of-dcpam-wds)
* [Technology](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#technology)
    * [Data Sources](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#data-sources)
    * [DCPAM Database](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#dcpam-database)
    * [Configuration](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#configuration)
    * [Compilation (Linux)](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#compilation-linux)
    * [Linux Dependencies](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#linux-dependencies)
    * [Windows Dependencies](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS#windows-dependencies)

## Purpose of DCPAM WDS
DCPAM WDS is dedicated endpoint for querying predefined business data with custom caching system. DCPAM does not limit number of virtual and physical DCPAM Database nodes, therefore DCPAM WDS encapsulates every node of DCPAM Database into single data access point.

## Technology
### Data sources
DCPAM WDS development is still in progress with following data sources available:
|  ID  | Data source                        | Type            | Support          |
|:----:|:-----------------------------------|:---------------:|:----------------:|
| 1    | PostgreSQL                         | database        | native           |
| 2    | MySQL 8                            | database        | native           |
| 3    | MariaDB/MySQL 5                    | database        | native           |
| 4    | SQL Server/Azure SQL Database      | database        | native via ODBC* |
| 5    | Oracle Database                    | database        | native           |
| 6    | SQLite3                            | database        | native           |

> \* SQL Server/Azure SQL Database: [ODBC is the primary native data access API for applications written in C and C++ for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server).

**Please note that DCPAM WDS provides support for every ODBC-compliant data source**.

### DCPAM Database
DCPAM is designed to be as most customizable as it needs to be.
Therefore every database listed above as available data source can also be used as DCPAM target database.

### Configuration
For DCPAM WDS `wds_config.json` is the main configuration file. It defines:
* DCPAM database.
* Tables and views (see _app.DATA_), where integrated data is stored.

#### Compilation (Linux)
> Before attempt to compile, please adjust `ORACLE_DEP` paths in `makefile`.
```
> make dcpam-wds
```
- run DCPAM WDS
```
> ./dcpam-wds
```
- run with Valgrind (with suppression of Oracle OCI errors) and default `wds_config.json`.
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_oci.supp ./dcpam-wds
```
- run with Valgrind and `wds_config_mysql.json` (where MySQL is DCPAM main database)
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no ./dcpam-wds wds_config_mysql.json
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

---
This software uses:
* [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.
* [SQLite3](https://www.sqlite.org/ "SQLite")
