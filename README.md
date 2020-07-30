# DCPAM [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
 
 Copyright © 2020 Marcin Kelar
###### _Data Construct-Populate-Access-Manage_ 
#### Data Warehouse Engine
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![Linux ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png)

* DCPAM helps to create central repositories of integrated data from one or disparate sources [[1]].
* DCPAM allows to perform advanced data copy between technically different datasets.
* DCPAM goal is to deliver full range of Data Warehouse possibilities without need to include or hire more engineers for this specific task.
* DCPAM architecture is highly flexible and provides unlimited scaling possibilities.
* DCPAM is multiplatform (Linux/Windows).

##### Currently under active development
* [x] Transform subprocess.

### Table of contents
* [Business Value](https://github.com/OrionExplorer/dcpam#business-value)
    * [Extraction, Staging and Change Data Capture](https://github.com/OrionExplorer/dcpam#extraction-staging-and-change-data-capture)
    * [Transformation](https://github.com/OrionExplorer/dcpam#transformation)
    * [Loading](https://github.com/OrionExplorer/dcpam#loading)
* [Data Warehouse with DCPAM](https://github.com/OrionExplorer/dcpam#data-warehouse-with-dcpam)
    * [What DCAM covers in terms of Data Warehousing?](https://github.com/OrionExplorer/dcpam#what-dcpam-covers-in-terms-of-data-warehousing)
    * [Elements yet to be covered by DCPAM](https://github.com/OrionExplorer/dcpam#elements-yet-to-be-covered-by-dcpam)
    * [Other](https://github.com/OrionExplorer/dcpam#other)
* [Technology](https://github.com/OrionExplorer/dcpam#technology)
    * [Architecture Overview](https://github.com/OrionExplorer/dcpam#architecture-overview)
    * [ETL and Change Data Capture](https://github.com/OrionExplorer/dcpam#etl-and-change-data-capture)
    * [Parallel Execution](https://github.com/OrionExplorer/dcpam#parallel-execution)
    * [Data Sources](https://github.com/OrionExplorer/dcpam#data-sources)
    * [DCPAM Database](https://github.com/OrionExplorer/dcpam#dcpam-database)
    * [Configuration](https://github.com/OrionExplorer/dcpam#configuration)
    * [Compilation (Linux)](https://github.com/OrionExplorer/dcpam#compilation-linux)
    * [Linux Dependencies](https://github.com/OrionExplorer/dcpam#linux-dependencies)
    * [Windows Dependencies](https://github.com/OrionExplorer/dcpam#windows-dependencies)
* [Roadmap](https://github.com/OrionExplorer/dcpam#roadmap)

## Business Value
* [x] **DCPAM helps to create single central repository of integrated company data** - this provides a single integrated view of an organisation.

* [x] **All informations are always up to date** - Managers can respond rapidly to ongoing changes in the business environment to make data-driven decisions.

* [x] **Data structures are designed in a uniform way** - much less effort is needed to prepare and access requested informations.

* [x] **No wide range of advanced technical knowledge is needed to make DCPAM work** - system configuration can be handled by any person with SQL background and analytic insight of company data.

* [x] **Process gigabytes of data within minutes** - benefit of parallel execution.


### Extraction, Staging and Change Data Capture
Extraction is first major process. Main DCPAM workflow consists of:
1. Data extraction from all source systems to the Staging Area or target tables directly.
2. (Optional) Data transformation using all source systems in the Staging Area.
3. Load transformed dataset to the target tables (when Staging Area is used).

![Main Overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/dwh.png)

#### Extraction
DCPAM is designed to perform both incremental and full online extraction without need to implement additional logic to the source system. This process is SQL-based all the way, thus precise configuration of various transaction logs is not required. Log scanning is great non-intrusive method for Change Data Capture, but DCPAM goal is to deliver full Data Warehouse possibilities without need to include or hire more engineers for this specific task.

Extract process does handle of:
1. **Extract Inserted** - find and fetch only new records.
2. **Extract Deleted** - find records that no longer exists in the source system.
3. **Extract Modified** - find records that have been modified since last extraction.

> **Information**: offline extraction (flat files) and other online sources will be available in the future.

Before Extract process begin, it is possible to run set of SQL queries to perform on DCPAM Database.

#### Staging
Depending on the configuration, extracted data is stored instantly either in the Staging Area or target tables directly. That means inserted, deleted and modified records from all source systems coexist in the transitional tables at the same time and must be properly marked. But this is another big performance boost: DCPAM can execute a number of simple SELECT queries instead of one complex SQL with many joins and other conditions, so impact on the source system is minimal.

Staging Area is a separate area in Data Warehouse, and DCPAM **does not require** it to be located in the same database or even on the same server.


#### Change Data Capture
DCPAM allows to define change tracking conditions to deliver near real-time or on-time data into Warehouse. Efficient identification of most recently changed data is crucial, but also most challenging. Successful implementation of change tracking has enormous impact on the size of data volume to be processed.
Two major techniques are used to track changed data:
1. **Timestamps**. Each table of the source system involved in Extract process *should* have timestamp column, where date and time of last modification is kept. This information stored in Warehouse is used to construct extract query.

2. **Row versioning**.

3. **Row status indicators**.

4. **Triggers**. These are created in the source system. Mentioned here for informational purposes only. Two possible use cases:
    * Set timestamp column values when record is modified.
    * Call external application/script with all necessary data (ie. with SQL User Defined Functions).

> **Notice**: Triggers can seriously affect performance of the source system, thus should be considered carefully.

### Transformation
Optionally, DCPAM stores extracted data in the Staging Area, where transformations can be applied. Staging Area is a group of transitional tables in DCPAM Database or external database server.

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

Moreover, Transform process should be able to perform backflow of cleaned data to the original source.

DCPAM can handle data transformations with two different approaches:
1. Locally - each transform module is executed on the same server, where DCPAM is.
2. Remotely - transform modules are located on one or more different machines. This is recommended solution for compute-intensive data transformations.

DCPAM does not limit the number of transformations in any way. Furthermore, both local and remote approaches can be used simultaneously.

Data transformation in DCPAM ETL workflow is not enforced.

> **Information**: This section is incomplete. Transform process in DCPAM is still under development.

### Loading
When all transformations in the Staging Area are completed or during the Extract subprocess, DCPAM loads the data directly into target tables. Dimensions are first to load, followed by Facts. Then Staging Area is cleared and ready for the next occurence of data extraction.

After Loading process is finished, it is possible to run set of SQL queries to perform on the DCPAM Database (ie. to clear Staging Area).

## Data Warehouse with DCPAM

### What DCPAM covers in terms of Data Warehousing?
* JSON-based source systems configuration.
* SQL and JSON-based configuration of the ETL processes:
  * [x] Data extraction:
    * [x] Inserted data
    * [x] Deleted data
    * [x] Modified data
  * [x] Staging Area:
    * [x] optional
    * [x] placed locally in DCPAM Database
    * [x] placed in external database
  * [ ] Data transformation:
    * [ ] optional
    * [ ] handled locally (in relation to Staging Area)
    * [ ] handled remotely (in relation to Staging Area)
  * [x] Data load from:
    * [x] local Staging Area
    * [x] remote Staging Area 
* Parallel execution:
  * [x] By design:
    * [x] Each ETL process runs in separate thread.
  * [x] By running multiple instances of DCPAM:
    * [x] On the same server
    * [x] On many disparate servers
* SQL and JSON-based preconfigured queries for data analysis.
* Data Warehouse and Data Marts:
  * One or many instances of DCPAM can work as Data Warehouse (extracting and processing data from disparate sources).
  * In the same time different DCPAM instances can use Data Warehouse to feed Data Marts with specific business-oriented data.
* Access DCPAM Database with any system for analytics (Power BI, Tableau, Redash etc.).

### Elements yet to be covered by DCPAM
* DCPAM BI web application:
  * SQL query exeution with grid-based results view.
  * Access to preconfigured queries.
  * Charts:
    * line
    * bar
    * pie
    * column
    * area
    * bubble
    * scatter
    * tree map
  * Manage custom reports:
    * labels, charts and grids
    * user access control
* DCPAM Access:
  * Caching mechanism for preconfigured queries.
* DCPAM Admin web application:
  * Manage data sources.
  * Configure ETL processes.
  * Manage DCPAM BI users.

### Other
* Choose Data Warehouse DBMS, sufficient hardware and disk space.
* Consider Data Warehouse tables schema:
  * Snowflake schema [[5]]
  * Star schema [[6]]
  * Galaxy schema
  * Fact constellation [[8]]
* Project data structures:
  * Staging Area
  * Target tables
  * Indexes
  * Views


## Technology
### Architecture Overview
* DCPAM offers either ETL [[2]] or ELT [[7]] solutions.
* Highly memory-efficient - no memory overhead caused by large queries:
  * each extracted record is instantly stored either in the Staging Area [[3]] or target tables directly by Extract process
  * each staged and transformed record is instantly loaded into target tables by Load process
* Staging Area:
  * fully supported in DCPAM, but not required in the ETL process
  * possibility to keep Staging Area outside the DCPAM Database
* Data transformation:
  * fully supported in DCPAM, but not required in the ETL process
  * possibility to run outside the DCPAM server
* Each Change Data Capture process operates independently in separate thread.
* Multiple instances of DCPAM with different configuration can run on single server.
* Database support is provided with native libraries (see _Data sources_ and _Linux Dependencies_ in this document).
* Simply put, DCPAM allows to perform advanced data copy between technically different datasets.

![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)

#### ETL and Change Data Capture
Change Data Capture [[4]] solutions depends on the data sources (currently it's database only):

| CDC solution                            | Source        |
|-----------------------------------------|:-------------:| 
| SQL query for timestamps (eg. MIN, MAX) | Database      |
| SQL query for indices                   | Database      |
| SQL query for diffs (eg. IN, NOT IN)    | Database      |

> **Notice**: Only Extract and Load processes are available. Transform process development is in progress.

### Parallel Execution
* It is possible to use multiple processes to accomplish single task. Instead of one configuration of many queries per source system, DCPAM allows to create many configurations of the same source system with one or more queries.
* Many instances of DCPAM can run on one or more servers simultaneously.

### Data sources
DCPAM development is still in progress with following data sources available:
|  ID  | Data source                        | Type            | Support          |
|:----:|:-----------------------------------|:---------------:|:----------------:|
| 1    | PostgreSQL                         | database        | native           |
| 2    | MySQL 8                            | database        | native           |
| 3    | MariaDB/MySQL 5                    | database        | native           |
| 4    | SQL Server/Azure SQL Database      | database        | native via ODBC* |
| 5    | Oracle Database                    | database        | native           |
| 6    | SQLite3                            | database        | native           |

> \* SQL Server/Azure SQL Database: [ODBC is the primary native data access API for applications written in C and C++ for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server).

**Please note that DCPAM provides support for every ODBC-compliant data source**.

### DCPAM Database
DCPAM is designed to be as most customizable as it needs to be.
Therefore every database listed above as available data source can also be used as DCPAM target database.

### Configuration
File `config.json` is default DCPAM foundation. It defines:
* Data sources.
* Extract, Stage, Transform and Load process for each data source.
* DCPAM database, tables and views (see _app.DATA_), where integrated data is stored.

More `json` files can be configured to achieve more flexibile and parallel ETL processes. Each `json` file is executed by new instance of DCPAM.


#### Compilation (Linux)
> Before attempt to compile, please adjust `ORACLE_DEP` paths in `makefile`.
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

## Roadmap
| 2020                              | 2021                                    |
|:----------------------------------|:----------------------------------------|
| Transform process                 | DCPAM Construct (admin web application) |
| DCPAM Populate (data caching)     | DCPAM AI platform                       |
| DCPAM Access (BI web application) |                                         |

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