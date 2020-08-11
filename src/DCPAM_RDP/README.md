# DCPAM ETL [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
 
 Copyright © 2020 Marcin Kelar
###### _Data Construct-Populate-Access-Manage_ 
#### Remote Data Processor
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png)

##### Currently under active development
* [ ] Nothing regarding DCPAM RDP.

### Table of contents
* [Purpose of DCPAM RDP](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_RDP#purpose-of-dcpam-rdp)
* [Technology](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#technology)
    * [Architecture Overview](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#architecture-overview)
    * [Compilation (Linux)](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#compilation-linux)
    * [Linux Dependencies](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#linux-dependencies)
    * [Windows Dependencies](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#windows-dependencies)

## Purpose of DCPAM RDP
DCPAM RDP is used by DCPAM-ETL to execute scripts/applications for data transformation located on the remote servers. It is often used when performance hit caused by these computations may impact other ongoing operations (ie. data extraction/load or user queries).

## Technology
### Architecture Overview
* Data transformation:
  * fully supported, but not required in the ETL/ELT process
  * possibility to run outside the DCPAM server

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/rdp.png)
Example of DCPAM Transform process scalability:
* Multiple DCPAM ETL engine nodes can be run within single Data Warehouse.
* Each DCPAM ETL instance can trigger unlimitend number of local and remote (through DCPAM RDP) data transformation scripts/applications.
* Each DCPAM ETL instance can use dedicated Staging Area node (local or remote).
* Many DCPAM Database nodes can be encapsulated into single data access point by DCPAM WDS.
* DCPAM ETL, Staging Area, DCPAM Database and DCPAM WDS can run on single server as well!

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