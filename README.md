# DCPAM [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master)
 
 [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade)
 
 Copyright © 2020 Marcin Kelar
###### _Data Construct-Populate-Access-Manage_ 
#### Data Warehouse
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![ODBC ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png) ![SQLite3 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![Linux ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10 ](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png)

* DCPAM helps to create central repositories of integrated data from one or disparate sources [[1]].
* DCPAM goal is to deliver full range of Data Warehouse possibilities without need to include or hire more engineers for this specific task.
* DCPAM architecture is [highly flexible](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#architecture-overview) and [provides unlimited scaling possibilities](https://github.com/OrionExplorer/dcpam#dcpam-rdp---remote-data-processor).
* DCPAM allows to perform advanced data copy between technically different datasets.
* DCPAM is multiplatform (Linux/Windows/Cloud).

![Main Overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/dwh.png)

### Table of contents
* [Business Value](https://github.com/OrionExplorer/dcpam#business-value)
    * [Extract, Transform and Load](https://github.com/OrionExplorer/dcpam#extract-transform-and-load)
* [Data Warehouse with DCPAM](https://github.com/OrionExplorer/dcpam#data-warehouse-with-dcpam)
    * [What DCPAM covers in terms of Data Warehousing?](https://github.com/OrionExplorer/dcpam#what-dcpam-covers-in-terms-of-data-warehousing)
    * [Elements yet to be covered by DCPAM](https://github.com/OrionExplorer/dcpam#elements-yet-to-be-covered-by-dcpam)
    * [Other](https://github.com/OrionExplorer/dcpam#other)
* [Technology](https://github.com/OrionExplorer/dcpam#technology)
    * [DCPAM Components](https://github.com/OrionExplorer/dcpam#dcpam-components)
      * [DCPAM ETL - Extract-Transform-Load / Extract-Load-Transform](https://github.com/OrionExplorer/dcpam#dcpam-etl---extract-transform-load--extract-load-transform)
      * [DCPAM WDS - Warehouse Data Server](https://github.com/OrionExplorer/dcpam#dcpam-wds---warehouse-data-server)
      * [DCPAM RDP - Remote Data Processor](https://github.com/OrionExplorer/dcpam#dcpam-rdp---remote-data-processor)
      * [DCPAM Construct](https://github.com/OrionExplorer/dcpam#dcpam-construct)
      * [DCPAM Access](https://github.com/OrionExplorer/dcpam#dcpam-access)
    * [Data Sources](https://github.com/OrionExplorer/dcpam#data-sources)
    * [DCPAM Database](https://github.com/OrionExplorer/dcpam#dcpam-database)
* [Roadmap](https://github.com/OrionExplorer/dcpam#roadmap)

## Business Value
* [x] **Open source, cost-effective**.

* [x] **DCPAM helps to create single central repository of integrated company data** - this provides a single integrated view of an organisation.

* [x] **All informations are always up to date** - Managers can respond rapidly to ongoing changes in the business environment to make data-driven decisions.

* [x] **Data structures are designed in a uniform way** - much less effort is needed to prepare and access requested informations.

* [x] **No wide range of advanced technical knowledge is needed to make DCPAM work** - installation is straightforward and system configuration can be handled by any person with SQL background and analytic insight of company data.

* [x] **Process gigabytes of data within minutes** - benefit of parallel execution.


### Extract, Transform and Load
For detailed information about ETL and ELT pipelines, please refer to [DCPAM ETL documentation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL).

More specific description of data transformation possibilities can be found in [DCPAM RDP documentation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_RDP).

## Data Warehouse with DCPAM

### What DCPAM covers in terms of Data Warehousing?
* JSON-based source systems configuration.
* SQL and JSON-based configuration of the ETL processes:
  * Data extraction:
    * Inserted data
    * Deleted data
    * Modified data
  * Staging Area:
    * optional
    * placed locally in DCPAM Database
    * placed in external database
  * Data transformation:
    * optional
    * handled locally (in relation to the Staging Area)
    * handled remotely (in relation to the Staging Area)
  * Data load from:
    * local Staging Area
    * remote Staging Area 
* Parallel execution:
  * By design:
    * Each ETL process runs in separate thread.
  * By running multiple instances of DCPAM:
    * On the same server
    * On many disparate servers
* SQL and JSON-based preconfigured queries for data analysis.
* Data Warehouse and Data Marts:
  * One or many instances of DCPAM can work as Data Warehouse (extracting and processing data from disparate sources).
  * In the same time different DCPAM instances can use Data Warehouse to feed Data Marts with specific business-oriented data.
* Data sources:
  * [Databases](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#data-sources)
* Access DCPAM Database with any system for analytics (Power BI, Tableau, Redash etc.).

### Elements yet to be covered by DCPAM
* DCPAM Access (BI web application):
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
* DCPAM WDS:
  * Caching mechanism for preconfigured queries.
* DCPAM Admin web application:
  * Manage data sources.
  * Configure ETL processes.
  * Manage DCPAM BI users.
* Data sources:
  * Local and remote flat files (csv, json)
  * LDAP
  * API

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
* ...and much more.


## Technology
### DCPAM Components
#### DCPAM ETL - Extract-Transform-Load / Extract-Load-Transform
[DCPAM ETL](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL) is the main ETL/ELT engine. Many instances of DCPAM ETL can work within single Data Warehouse.

#### DCPAM WDS - Warehouse Data Server
[DCPAM WDS](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS) is dedicated endpoint for querying predefined business data. It is custom caching system for DCPAM.

#### DCPAM RDP - Remote Data Processor
[DCPAM RDP](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_RDP) is used by DCPAM ETL to execute transform scripts/applications that must be run on separate machines when performance impact is significant. DCPAM ETL communicate with these remote scripts/applications through DCPAM RDP.

![DCPAM Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)
*<p align=center>DCPAM workflow overview</p>*


![DCPAM Transform process architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/rdp.png)
Example of DCPAM Transform process scalability:
* Multiple DCPAM ETL engine nodes can be run within single Data Warehouse.
* Each DCPAM ETL instance can trigger unlimitend number of local and remote (through DCPAM RDP) data transformation scripts/applications.
* Each DCPAM ETL instance can use dedicated Staging Area node (local or remote).
* Many DCPAM Database nodes can be encapsulated into single data access point by DCPAM WDS.
* DCPAM ETL, Staging Area, DCPAM Database and DCPAM WDS can run on single server as well!

#### DCPAM Construct
DCPAM Construct is going to be the main system administration web application.

#### DCPAM Access
DCPAM Access is going to be dedicated Bussiness Inteligence web application.

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

##### Currently under active development
* [x] [DCPAM WDS](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS).

## Roadmap
| 2020                              | 2021                                    |
|:----------------------------------|:----------------------------------------|
| ~Transform process~               | DCPAM Construct (admin web application) |
| DCPAM WDS (warehouse data server) | DCPAM AI platform                       |
| DCPAM Access (BI web application) |                                         |

---
This software uses:
* [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.
* [SQLite3](https://www.sqlite.org/ "SQLite")

[1]: https://en.wikipedia.org/wiki/Data_warehouse
[5]: https://en.wikipedia.org/wiki/Snowflake_schema
[6]: https://en.wikipedia.org/wiki/Star_schema
[8]: https://en.wikipedia.org/wiki/Fact_constellation