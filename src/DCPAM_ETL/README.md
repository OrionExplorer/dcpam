# DCPAM ETL [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
 
 Copyright © 2020 Marcin Kelar
###### _Data Construct-Populate-Access-Manage_ 
#### Extract-Transform-Load Engine
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![CSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/csv100x100.png) ![JSON](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/json100x100.png) ![API](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/api100x100.png) ![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png) ![Cloud](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/cloud100x100.png)

##### Currently under active development
* [x] Data source: CSV.

### Table of contents
* [Extraction, Staging and Change Data Capture](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#extraction-staging-and-change-data-capture)
* [Transformation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#transformation)
* [Loading](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#loading)
* [Technology](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#technology)
    * [Architecture Overview](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#architecture-overview)
    * [ETL and Change Data Capture](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#etl-and-change-data-capture)
    * [Parallel Execution](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#parallel-execution)
    * [Data Sources](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#data-sources)
      * [Databases](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#databases)
      * [Flat Files](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#flat-files)
    * [DCPAM Database](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#dcpam-database)
    * [Configuration](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#configuration)
    * [Compilation (Linux)](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#compilation-linux)
    * [Linux Dependencies](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#linux-dependencies)
    * [Windows Dependencies](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#windows-dependencies)

## Extraction, Staging and Change Data Capture
Extraction is first major process. Main DCPAM ETL/ELT workflow consists of:
1. Data extraction from all source systems to the Staging Area or target tables directly.
2. (Optional) Data transformation using all source systems in the Staging Area.
3. Load transformed dataset to the target tables (when Staging Area is used).

![Main Overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/dwh.png)

#### Extraction
DCPAM ETL is designed to perform both incremental and full online/offline extraction without need to implement additional logic to the source system. This process is SQL-based all the way, thus precise configuration of various transaction logs is not required. Log scanning is great non-intrusive method for Change Data Capture, but DCPAM goal is to deliver full Data Warehouse possibilities without need to include or hire more engineers for this specific task.

Extract process does handle of:
1. **Extract Inserted** - find and fetch only new records.
2. **Extract Deleted** - find records that no longer exists in the source system.
3. **Extract Modified** - find records that have been modified since last extraction.

> **Information**: other online sources will be available in the future.

Before Extract process begin, it is possible to run set of SQL queries to perform on DCPAM Database.

#### Staging
Depending on the configuration, extracted data is stored instantly either in the Staging Area or target tables directly. That means inserted, deleted and modified records from all source systems coexist in the transitional tables at the same time and must be properly marked. But this is another big performance boost: DCPAM ETL can execute a number of simple SELECT queries instead of one complex SQL with many joins and other conditions, so impact on the source system is minimal.

Staging Area is a separate area in Data Warehouse, and DCPAM ETL **does not require** it to be located in the same database or even on the same server.


#### Change Data Capture
DCPAM ETL allows to define change tracking conditions to deliver near real-time or on-time data into Warehouse. Efficient identification of most recently changed data is crucial, but also most challenging. Successful implementation of change tracking has enormous impact on the size of data volume to be processed.

Below are listed some of the major techniques used to track changed data:
1. **Timestamps**. Each table of the source system involved in Extract process *should* have timestamp column, where date and time of last modification is kept. This information stored in Warehouse is used to construct extract query.

2. **Row versioning**.

3. **Row status indicators**.

4. **Triggers**. These are created in the source system. Mentioned here for informational purposes only. Two possible use cases:
    * Set timestamp column values when record is modified.
    * Call external application/script with all necessary data (ie. with SQL User Defined Functions).

> **Notice**: Triggers can seriously affect performance of the source system, thus should be considered carefully.

### Transformation
Optionally, DCPAM ETL stores extracted data in the Staging Area, where transformations can be applied. Staging Area is a group of transitional tables in DCPAM Database or external database server.

Sample operations to perform with this process:
* data merging
* data cleansing
* data validations and corrections
* conversions
* aggregations
* recalculations
* column values completions
* creation of entirely new views
* data enrichments

Moreover, Transform process is able to perform backflow of cleaned data to the original source.

DCPAM ETL can handle data transformations with two different approaches:
1. **Locally** - each transform module is executed on the same server, where DCPAM is.
2. **Remotely** - transform modules are located on one or more different machines and operated by [DCPAM RDP](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_RDP). This is recommended solution for compute-intensive data transformations.

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/rdp.png)

DCPAM ETL does not limit the number of transformations in any way. Furthermore, both local and remote approaches can be used simultaneously.

Data transformation in DCPAM ETL pipeline is not enforced.



### Loading
When all transformations in the Staging Area are completed or during the Extract subprocess, DCPAM ETL loads the data directly into target tables. Dimensions are first to load, followed by Facts. Then Staging Area is cleared and ready for the next occurence of data extraction.

After Loading process is finished, it is possible to run set of SQL queries to perform on the DCPAM Database (ie. to clear Staging Area).

## Technology
### Architecture Overview
* DCPAM ETL offers either ETL [[2]] or ELT [[7]] based-solutions.
* Highly memory-efficient - no memory overhead caused by large queries:
  * each extracted record is instantly stored either in the Staging Area [[3]] or target tables directly by Extract process
  * each staged and transformed record is instantly loaded into target tables by Load process
* Highly scalable:
  * multiple instances of DCPAM ETL with different configuration can run on single server
  * many of the DCPAM ETL instances can run across many servers
  * data transformations can run across many servers
* Staging Area:
  * fully supported, but not required in the ETL/ELT process
  * possibility to keep Staging Area outside the DCPAM Database
* Data transformation:
  * fully supported, but not required in the ETL/ELT process
  * possibility to run outside the DCPAM server
* Each ETL/ELT process operates independently in separate thread.
* Database support is provided with native libraries (see _Data sources_ and _Linux Dependencies_ in this document).
* Simply put, DCPAM ETL allows to perform advanced data copy between technically different datasets.

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)

#### ETL and Change Data Capture
Change Data Capture [[4]] solutions depends on the data sources and their quality.

| CDC solution                            | Source        |
|-----------------------------------------|:-------------:| 
| SQL query for timestamps (eg. MIN, MAX) | database/file |
| SQL query for indices                   | database/file |
| SQL query for diffs (eg. IN, NOT IN)    | database/file |


### Parallel Execution
* It is possible to use multiple processes to accomplish single task. Instead of one configuration of many queries per source system, DCPAM ETL allows to create many configurations of the same source system with one or more queries.
* Many instances of DCPAM ETL can run on one or more servers simultaneously.

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

> \* SQL Server/Azure SQL Database: [ODBC is the primary native data access API for applications written in C and C++ for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server).

**Please note that DCPAM provides support for every ODBC-compliant data source**.

#### Flat Files
| Data source                        | Support          |
|:-----------------------------------|:----------------:|
| CSV                                | native           |
| JSON                               | native           |

DCPAM can access files from local or remote locations. The latter are fetched via HTTP protocol.

Files are loaded to temporary tables in DCPAM or external database to make SQL operations possible for this kind of data.

### DCPAM Database
DCPAM is designed to be as most customizable as it needs to be.
Therefore every database listed above as available data source can also be used as DCPAM target database.

### Configuration
For DCPAM ETL `etl_config.json` is the main configuration file. It defines:
* DCPAM database.
* Data sources.
* Extract, Stage, Transform and Load process for each data source.
* ETL or ELT mode.
* Pre- and PostETL actions.

More than one `json` configuration file can be configured to achieve more flexibile and parallel execution of configured processes. Each `json` file is executed by new instance of DCPAM ETL module.


#### Compilation (Linux)
> Before attempt to compile, please adjust `ORACLE_DEP` paths in `makefile`.
```
> make dcpam-etl
```
- run DCPAM ETL
```
> ./dcpam-etl
```
- run with Valgrind (with suppression of Oracle OCI errors) and default `etl_config.json`.
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_oci.supp ./dcpam-etl
```
- run with Valgrind and `etl_config_mysql.json` (where MySQL is DCPAM main database)
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no ./dcpam-etl etl_config_mysql.json
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