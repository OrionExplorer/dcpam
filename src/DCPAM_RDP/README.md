# DCPAM RDP - Remote Data Processor
![CodeQL](https://github.com/OrionExplorer/dcpam/workflows/CodeQL/badge.svg) ![C/C++ CI](https://github.com/OrionExplorer/dcpam/workflows/C/C++%20CI/badge.svg) [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade) [![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-brightgreen.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
##### Copyright © 2020 - 2021 Marcin Kelar  
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![IBM Db2](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ibmdb2100x100.png) ![MongoDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mongodb105x100.png) ![XLSX](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/xlsx100x100.png) ![XML](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/xml100x100.png) ![ODS](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ods100x100.png) ![CSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/csv100x100.png) ![TSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/tsv100x100.png) ![PSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/psv100x100.png) ![JSON](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/json100x100.png) ![API](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/api100x100.png) ![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png) ![Cloud](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/cloud100x100.png) ![Docker](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/docker176x100.png)

##### Currently under active development
* [ ] To be announced.

### Table of contents
* [Purpose of DCPAM RDP](#purpose-of-dcpam-rdp)
* [Technology](#technology)
    * [Architecture Overview](#architecture-overview)
    * [Docker image](#docker-image)
    * [Compilation (Linux)](#compilation-linux)
    * [Linux Dependencies](#linux-dependencies)
    * [Windows Dependencies](#windows-dependencies)

## Purpose of DCPAM RDP
DCPAM RDP is used by DCPAM-ETL to execute scripts/applications for data transformation located on the remote servers. It is often used when performance hit caused by these computations may impact other ongoing operations (ie. data extraction/load or user queries).

## Technology
### Architecture Overview
* Server application dedicated for DCPAM ETL requests.
* Connections accepted from allowed hosts only with valid API key.
* DCPAM ETL provides all the information about Staging Area and source system databases, so backflow of cleaned data is possible.

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/rdp.png)
Example of DCPAM Transform process scalability:
* Multiple DCPAM ETL engine nodes can be run within single Data Warehouse.
* Each DCPAM ETL instance can trigger unlimitend number of local and remote (through DCPAM RDP) data transformation scripts/applications.
* Each DCPAM ETL instance can use dedicated Staging Area node (local or remote).
* Many DCPAM Database nodes can be encapsulated into single data access point by DCPAM WDS.
* DCPAM ETL, Staging Area, DCPAM Database and DCPAM WDS can run on single server as well!

#### Docker image
> Build DCPAM RDP image `dcpam-rdp`:
```
> docker build -t dcpam-rdp:latest . --file dcpam-rdp.dockerfile
```
> Run DCPAM RDP with mapped `rdp_config.json`:
```
> docker run -ti -v /home/dcpam/conf/rdp_config.json:/opt/dcpam/conf/rdp_config.json --rm dcpam-rdp:latest
```

#### Compilation (Linux)
> Before attempt to compile, please adjust `ORACLE_DEP` paths in `makefile`.
```
> make dcpam-rdp
```
- run DCPAM RDP
```
> ./dcpam-rdp
```
- run with Valgrind (with suppression of Oracle OCI errors)
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_oci.supp ./dcpam-rdp
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

[1]: https://en.wikipedia.org/wiki/Data_warehouse
[2]: https://en.wikipedia.org/wiki/Extract,_transform,_load
[3]: https://en.wikipedia.org/wiki/Staging_(data)
[4]: https://en.wikipedia.org/wiki/Change_data_capture
[5]: https://en.wikipedia.org/wiki/Snowflake_schema
[6]: https://en.wikipedia.org/wiki/Star_schema
[7]: https://en.wikipedia.org/wiki/Extract,_load,_transform
[8]: https://en.wikipedia.org/wiki/Fact_constellation