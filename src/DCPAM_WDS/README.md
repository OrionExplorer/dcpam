# DCPAM WDS - Warehouse Data Server
![CodeQL](https://github.com/OrionExplorer/dcpam/workflows/CodeQL/badge.svg) ![C/C++ CI](https://github.com/OrionExplorer/dcpam/workflows/C/C++%20CI/badge.svg) [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade) [![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-brightgreen.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
##### Copyright © 2020 - 2021 Marcin Kelar  
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![IBM Db2](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ibmdb2100x100.png) ![MongoDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mongodb105x100.png) ![XLSX](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/xlsx100x100.png) ![MSACCESS](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/msaccess100x100.png) ![XML](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/xml100x100.png) ![ODS](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ods100x100.png) ![CSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/csv100x100.png) ![TSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/tsv100x100.png) ![PSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/psv100x100.png) ![JSON](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/json100x100.png) ![API](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/api100x100.png) ![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png) ![Cloud](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/cloud100x100.png)

##### Currently under active development
* [x] Cache TTL.

### Table of contents
* [Purpose of DCPAM WDS](#purpose-of-dcpam-wds)
* [Technology](#technology)
    * [Architecture Overview](#architecture-overview)
    * [Data Sources](#data-sources)
        * [Databases](#databases)
        * [Flat Files](#flat-files)
        * [API](#api)
    * [DCPAM Database](#dcpam-database)
    * [Configuration](#configuration)
    * [Docker image](#docker-image)
    * [Compilation (Linux)](#compilation-linux)
    * [Linux Dependencies](#linux-dependencies)
    * [Windows Dependencies](#windows-dependencies)

## Purpose of DCPAM WDS
[DCPAM WDS](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS/README.md) is dedicated [Massively Parallel Processing](https://pl.wikipedia.org/wiki/MPP) service for querying predefined and custom business data with in-memory caching system. DCPAM WDS encapsulates every node of DCPAM Database into single data access point.

## Technology
### Architecture Overview
* Server application dedicated for the backends of client applications.
* Snowflake schema with single or multiple access points.
* Massively Parallel Processing architecture.
* Connections accepted from allowed hosts with valid API key only.
* JSON-based communication with client applications.
* Builds the in-memory cache for queries predefined in `wds_config.json` on startup:
    * For one or more DCPAM Database nodes.
* Caches data on the fly for new requests.
* Maximum allowed memory usage configuration.
* May need a lot of RAM.

### Data sources
DCPAM development is still in progress with following data sources available:

#### Databases
| Data source                        | Support          |
|:-----------------------------------|:----------------:|
| PostgreSQL                         | native           |
| MySQL 8                            | native           |
| MariaDB/MySQL 5                    | native           |
| SQL Server/Azure SQL Database      | native via ODBC* |
| Oracle Database                    | native           |
| SQLite3                            | native           |
| IBM Db2                            | ODBC             |
| Microsoft Access                   | ODBC             |
| MongoDB                            | native           |

> \* SQL Server/Azure SQL Database: [ODBC is the primary native data access API for applications written in C and C++ for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server).

**Please note that DCPAM provides support for every ODBC-compliant data source**.

#### Flat Files
| Data source                        | Support          |
|:-----------------------------------|:----------------:|
| CSV                                | native           |
| TSV                                | native           |
| PSV                                | native           |
| JSON                               | native           |
| XLS                                | [script](https://github.com/OrionExplorer/dcpam/blob/master/src/DCPAM_ETL/data-processor/excel2csv.py)
| XLSX                               | script           |
| XLSM                               | script           |
| XLSB                               | script           |
| ODF                                | script           |
| ODS                                | script           |
| XML                                | [script](https://github.com/OrionExplorer/dcpam/blob/master/src/DCPAM_ETL/data-processor/xml2csv.py) |

#### API
DCPAM can access files from local or remote locations. The latter are fetched via HTTP/HTTPS protocol and [Battery HTTP Server](https://github.com/OrionExplorer/battery-http-server) is recommended.
Parameters such as URL, method, headers and additional payload are supported.
Files are loaded to temporary tables in DCPAM or external database to make SQL operations possible for this kind of data.

### DCPAM Database
DCPAM is designed to be as most customizable as it needs to be.
Therefore every database listed above as available data source can also be used as DCPAM target database.

### Configuration
For DCPAM WDS `wds_config.json` is the main configuration file. It defines:
* DCPAM database.
* Tables and views (see _app.DATA_), where integrated data is stored.

#### Docker image
> Build DCPAM WDS image `dcpam-wds`:
```
> docker build -t dcpam-wds:latest . --file dcpam-wds.dockerfile
```
> Run DCPAM WDS with mapped `wds_config.json`:
```
> docker run -ti -v /home/dcpam/conf/wds_config.json:/opt/dcpam/conf/wds_config.json --rm dcpam-wds:latest
```

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
- libmongoc + libbson (MongoDB)
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
