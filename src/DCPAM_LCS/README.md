# DCPAM LCS - Live Component State
[![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade) [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)
##### Copyright © 2020 Marcin Kelar
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![IBM Db2](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ibmdb2100x100.png) ![XLSX](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/xlsx100x100.png) ![ODS](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ods100x100.png) ![CSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/csv100x100.png) ![TSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/tsv100x100.png) ![PSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/psv100x100.png) ![JSON](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/json100x100.png) ![API](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/api100x100.png) ![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png) ![Cloud](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/cloud100x100.png) ![Docker](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/docker176x100.png)

##### Currently under active development
* [ ] To be announced. 

### Table of contents
* [Purpose of DCPAM LCS](#purpose-of-dcpam-lcs)
* [Technology](#technology)
    * [Architecture Overview](#architecture-overview)
    * [Configuration](#configuration)
    * [Docker image](#docker-image)
    * [Compilation (Linux)](#compilation-linux)

## Purpose of DCPAM LCS
DCPAM LCS is integrated repository of information about DCPAM Data Warehouse operational state. Each valid DCPAM module is obliged to send reports about ongoing operations directly to DCPAM LCS:
* DCPAM ETL - current Extract, Staging, Transform and Load operation status.
* DCPAM RDP - ongoing Transform scripts execution data.
* DCPAM WDS - memory usage, running queries and connected users stats.

DCPAM Components are also required by DCPAM LCS to respond to internal ping requests. 
Therefore **DCPAM Live Component State is the only module to assure Data Warehouse operability**.

## Technology
### Architecture Overview
* Server and client modules:
	* Server:
		* dedicated for reports sent by DCPAM Components (DCPAM Component -> DCPAM LCS).
		* dedicated for backends of client applications (ie. DCPAM Monitoring) for data presentation
	* Client:
		* for sending internal ping requests to all of the DCPAM Components (DCPAM LCS -> DCPAM Component).

### Configuration
For DCPAM LCS `lcs_config.json` is the main configuration file. It defines:
* Network configuration.
* DCPAM Component list required for DCPAM Data Warehouse to work.

#### Docker image
> Build DCPAM LCS image `dcpam-lcs`:
```
> docker build -t dcpam-lcs:latest . --file dcpam-lcs.dockerfile
```
> Run DCPAM LCS with mapped `lcs_config.json`:
```
> docker run -ti -v /home/dcpam/conf/lcs_config.json:/opt/dcpam/conf/lcs_config.json --rm dcpam-lcs:latest
```

#### Compilation (Linux)
> Before attempt to compile, please adjust `ORACLE_DEP` paths in `makefile`.
```
> make dcpam-lcs
```
- run DCPAM LCS
```
> ./dcpam-lcs
```
- run with Valgrind
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no ./dcpam-lcs
```

---
This software uses:
* [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.
