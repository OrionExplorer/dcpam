# DCPAM [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
###### _Data Construct-Populate-Access-Manage_ 
#### Data Warehouse Engine
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![Linux ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png)

* DCPAM helps to create central repositories of integrated data from one or disparate sources[[1]].
* DCPAM allows to perform advanced data copy between technically different datasets.
* DCPAM goal is to deliver full range of Data Warehouse possibilities and not include or hire more engineers for this specific task.
* DCPAM is multiplatform (Linux/Windows).

### Table of content
* [Business Value](https://github.com/OrionExplorer/dcpam#business-value)
    * [Extraction and Change Data Capture](https://github.com/OrionExplorer/dcpam#extraction-and-change-data-capture)
    * [Transformation](https://github.com/OrionExplorer/dcpam#transformation)
    * [Loading](https://github.com/OrionExplorer/dcpam#loading)
* [Technology](https://github.com/OrionExplorer/dcpam#technology)
    * [Architecture Overview](https://github.com/OrionExplorer/dcpam#architecture-overview)
    * [ETL and Change Data Capture](https://github.com/OrionExplorer/dcpam#etl-and-change-data-capture)
    * [Data Sources](https://github.com/OrionExplorer/dcpam#data-sources)
    * [DCPAM Database](https://github.com/OrionExplorer/dcpam#dcpam-database)
    * [Configuration](https://github.com/OrionExplorer/dcpam#configuration)
    * [Compilation (Linux)](https://github.com/OrionExplorer/dcpam#compilation-linux)
    * [Linux Dependencies](https://github.com/OrionExplorer/dcpam#linux-dependencies)
    * [Windows Dependencies](https://github.com/OrionExplorer/dcpam#windows-dependencies)

## Business Value
* [x] **DCPAM helps to create single central repository of integrated company data** - this provides a single integrated view of an organisation.

* [x] **All informations  are always up to date** - Managers can respond rapidly to ongoing changes in the business environment to make data-driven decisions.

* [x] **Data structures are designed in a uniform way** - much less effort is needed to prepare and access requested informations.

* [x] **No wide range of advanced technical knowledge is needed to make DCPAM work** - system configuration can be handled by any person with SQL background and analytic insight of company data.

### Extraction and Change Data Capture
#### Extraction
DCPAM is designed to perform online incremental extraction without need to implement additional logic to the source system. This process is SQL-based all the way, thus precise configuration of various databases transaction logs are not required. Log scanning is great non-intrusive method for Change Data Capture, but DCPAM goal is to deliver full Data Warehouse possibilities and not include or hire more engineers for this specific task.

Extract process does handle of:
1. **Extract Inserted** - find and fetch only new records.
2. **Extract Deleted** - find records that no longer exists in the source system.
3. **Extract Modified** - find records that were modified since last extraction.

> **Information**: offline extraction (flat files) and other online sources will be available in the future.

Extracted data is stored in the Staging Area, where transformations can be applied.

#### Change Data Capture
DCPAM allows to define change tracking conditions to deliver near real-time or on-time data into Warehouse. Efficient identification of most recently changed data is crucial, but also most challenging. Successful implementation of change tracking has enormous impact on the size of data volume to be processed.
Two major techniques are used to track changed data:
1. **Timestamps**. Each table of the source system involved in Extract process *should* have timestamp column, where date and time of last modification is kept. This information stored in Warehouse is used to construct extract query.

2. **Triggers**. These are created in the source system. Mentioned here for informational purposes only. Two possible use cases:
    * Set timestamp column values when record is modified.
    * Call external application/script with all necessary data (ie. UDF in MySQL/MariaDB)

> **Notice**: Triggers can seriously affect performance of the source system, thus should be considered carefully.

### Transformation
DCPAM stores extracted data in the Staging Area - a group of transitional tables, where transformations are made.
Examples of data transformation:
* conversions
* recalculations
* column values completions
> **Information**: This section is incomplete. Transform process in DCPAM is yet to be implemented.

### Loading
When all transformations in the Staging Area are completed, DCPAM loads the data directly into target tables. Then Staging Area is cleared and ready for the next occurence of data extraction.

## Technology
### Architecture Overview
* DCPAM is ETL - based solution[[2]].
* Highly memory-efficient - no memory overhead caused by large queries:
	* each extracted record is instantly stored into Staging Area[[3]] by Extract process
	* each staged and transformed record is instantly loaded into target tables by Load process
* Each Change Data Capture process operates independently within separated thread.
* Multiple instances of DCPAM with different configuration can run on single server.
* Database support is provided with native libraries (see _Data sources_ and _Linux Dependencies_ in this document).
* Simply put, DCPAM allows to perform advanced data copy between technically different datasets.

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)

#### ETL and Change Data Capture
Change Data Capture[[4]] solutions depends on the data sources (currently it's database only):

| CDC solution                            | Source        |
|-----------------------------------------|:-------------:| 
| SQL query for timestamps (eg. MIN, MAX) | Database      |
| SQL query for diffs (eg. IN, NOT IN)    | Database      |


> **Notice**: Only Extract and Load processes are available. It is yet to be decided how to handle Transform process.

### Data sources
DCPAM development is still in progress with following data sources available:
|  ID  | Data source                        | Type            | Support          |
|:----:|:-----------------------------------|:---------------:|:----------------:|
| 1    | PostgreSQL      					| database        | native           |
| 2    | MySQL 8         					| database        | native           |
| 3    | MariaDB/MySQL 5 					| database        | native           |
| 4    | SQL Server/Azure SQL Database      | database        | native via ODBC* |
| 5    | Oracle Database 					| database        | native           |
| 6    | SQLite3         					| database        | native           |

> \* SQL Server/Azure SQL Database: [ODBC is the primary native data access API for applications written in C and C++ for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server).

### DCPAM Database
DCPAM is designed to be as most customizable as it needs to be.
Therefore every database listed above as available data source can also be used as DCPAM target database.

### Configuration
File `config.json` is DCPAM foundation. It defines:
* Data sources
* Extract, Transform and Load process for each data source
* DCPAM database, tables and views (see _app.DATA_), where integrated data is stored


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


This software uses:
* [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.
* [SQLite3](https://www.sqlite.org/ "SQLite")

[1]: https://en.wikipedia.org/wiki/Data_warehouse
[2]: https://en.wikipedia.org/wiki/Extract,_transform,_load
[3]: https://en.wikipedia.org/wiki/Staging_(data)
[4]: https://en.wikipedia.org/wiki/Change_data_capture
