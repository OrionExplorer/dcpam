# DCPAM - Data Warehouse Solution
[![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam) [![Build status](https://ci.appveyor.com/api/projects/status/43le8rn6721j8jtj/branch/master?svg=true)](https://ci.appveyor.com/project/OrionExplorer/dcpam/branch/master) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f5c3afcc56ab4e14910d7f68038d732a)](https://www.codacy.com/manual/OrionExplorer/dcpam?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OrionExplorer/dcpam&amp;utm_campaign=Badge_Grade) [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)
##### Copyright © 2020 Marcin Kelar
### Data Construct-Populate-Access-Manage
* DCPAM goal is to deliver full range of Data Warehouse possibilities without need to include or hire more engineers for this task.
* DCPAM architecture is [highly flexible](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL#architecture-overview) and [provides unlimited scaling possibilities](#dcpam-rdp---remote-data-processor).
* DCPAM is multiplatform solution: on-premise (Linux/Windows), cloud (Microsoft Azure, Amazon AWS, Google Cloud Platform) and hybrid delpoyments are possible.

### Supported databases
![PostgreSQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/postgresql102x100.png) ![MySQL](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mysql159x100.png) ![MariaDB](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/mariadb100x100.png) ![Microsoft SQL Server](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlserver134x100.png) ![Oracle Database](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/oracle100x100.png) ![SQLite3](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/sqlite171x100.png) ![IBM Db2](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/ibmdb2100x100.png) ![ODBC](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/odbc199x100.png)
### Other supported sources
![XLSX](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/xlsx100x100.png) ![CSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/csv100x100.png) ![TSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/tsv100x100.png) ![PSV](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/psv100x100.png) ![JSON](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/json100x100.png) ![API](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/api100x100.png)
### Deployment
![Linux](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/linux100x100.png) ![Windows 10](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/windows87x100.png) ![Cloud](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/cloud100x100.png) ![Docker](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/docker176x100.png) ![Microsoft Azure](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/azure256x100.png) ![Google Cloud Platform](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/google162x100.png) ![Amazon AWS](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/amazon236x100.png)

### Table of contents
* [Business Value](#business-value)
    * [Company data in DCPAM](#company-data-in-dcpam)
* [Data Warehouse with DCPAM](#data-warehouse-with-dcpam)
    * [What DCPAM covers in terms of Data Warehousing?](#what-dcpam-covers-in-terms-of-data-warehousing)
    * [Elements yet to be covered by DCPAM](#elements-yet-to-be-covered-by-dcpam)
    * [Other](#other)
* [Technology](#technology)
    * [DCPAM Components](#dcpam-components)
      * [DCPAM ETL - Extract-Transform-Load / Extract-Load-Transform](#dcpam-etl---extract-transform-load--extract-load-transform)
      * [DCPAM WDS - Warehouse Data Server](#dcpam-wds---warehouse-data-server)
      * [DCPAM RDP - Remote Data Processor](#dcpam-rdp---remote-data-processor)
      * [DCPAM LCS - Live Component State](#dcpam-lcs---live-component-state)
      * [DCPAM Construct](#dcpam-construct)
      * [DCPAM Access](#dcpam-access)
      * [DCPAM Monitoring](#dcpam-monitoring)
    * [Data Sources](#data-sources)
      * [Databases](#databases)
      * [Flat Files](#flat-files)
    * [DCPAM Database](#dcpam-database)
    * [Docker images](#docker-images)
* [Roadmap](#roadmap)

## Business Value
* [x] **Open source, cost-effective**.

* [x] **DCPAM helps to create single central repository of integrated company data** - this provides a single integrated view of an organisation.

* [x] **All informations are always up to date** - Managers can respond rapidly to ongoing changes in the business environment to make data-driven decisions.

* [x] **Data structures are designed in a uniform way** - much less effort is needed to prepare and access requested informations.

* [x] **No wide range of advanced technical knowledge is needed to make DCPAM work** - installation is straightforward and system configuration can be handled by any person with SQL background and analytic insight of company data.

* [x] **Process gigabytes of data within minutes** - benefit of parallel execution.

* [x] **Flexible deployment** - use your own infrastructure or get the benefits from cloud platforms (Microsoft Azure, Amazon AWS, Google Cloud Platform).

![Main Overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/dwh.png)

### Company data in DCPAM
DCPAM is responsible for copying data from one or more sources into a destination system. That process consists of three steps: Extraction, Transformation and Load, with Transform and Load in various configurations: Extract-Transform-Load (ETL) or Extract-Load-Transform (ELT).
* For detailed information about ETL and ELT pipelines, please refer to [DCPAM ETL documentation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL/README.md).
* More specific description of data transformation possibilities can be found in [DCPAM RDP documentation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_RDP/README.md).

Data in the Warehouse can be accessed directly at the database level with any system for analytics, such as Power BI, Tableau, Redash and others.
For complex architectures with more than one DCPAM Database node, DCPAM Warehouse Data Server is dedicated data access point. For more details, please refer to [DCPAM WDS documentation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS/README.md).

All of the DCPAM Data Warehouse operations are monitored by DCPAM Live Component State. See the [documentation](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_LCS/README.md) to read more on this subject.

## Data Warehouse with DCPAM
As DCPAM is extremely modular and highly scalabe, it can serve both as Data Warehouse and number of dedicated Data Marts.

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
    * results of the Extract subprocess
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
  * [Databases](#data-sources)
  * [Flat Files](#flat-files)
* Data Warehouse monitoring:
  * DCPAM ETL
  * DCPAM RDP
  * DCPAM WDS
  * DCPAM LCS
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
* DCPAM Admin web application:
  * Manage data sources.
  * Configure ETL processes.
  * Manage DCPAM BI users.
* DCPAM Monitoring (web application for DCPAM LCS).
* DCPAM LCS Notifications
* Data sources:
  * XML
  * LDAP

### Other
* Choose Data Warehouse DBMS, sufficient hardware and disk space.
* Consider Data Warehouse tables schema:
  * Snowflake schema [[1]]
  * Star schema [[2]]
  * Galaxy schema
  * Fact constellation [[3]]
* Project data structures:
  * Staging Area
  * Target tables
  * Indexes
  * Views
* ...and much more.

## Technology
### DCPAM Components
DCPAM Data Warehouse Solution consists of a number of integrated components. Each component has unique log file, and DCPAM ETL - due to parallel execution - can create many log files simultaneously.

#### DCPAM ETL - Extract-Transform-Load / Extract-Load-Transform
[DCPAM ETL](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_ETL/README.md) is the main ETL/ELT engine. Many instances of DCPAM ETL can work within single Data Warehouse.

#### DCPAM WDS - Warehouse Data Server
[DCPAM WDS](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_WDS/README.md) is dedicated endpoint for querying predefined business data with custom caching system. DCPAM WDS encapsulates every node of DCPAM Database into single data access point.

#### DCPAM RDP - Remote Data Processor
[DCPAM RDP](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_RDP/README.md) is used by DCPAM ETL to execute transform scripts/applications that must be run on separate machines when performance impact is significant. DCPAM ETL communicates with these remote scripts/applications through DCPAM RDP.

![DCPAM Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)
*<p align=center>DCPAM workflow overview</p>*

![DCPAM Transform process architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/rdp.png)
Example of DCPAM Extract-Transform-Load / Extract-Load-Transform process scalability:
* Multiple DCPAM ETL engine nodes can be run within single Data Warehouse.
* Each DCPAM ETL instance can trigger unlimitend number of local and remote (through DCPAM RDP) data transformation scripts/applications.
* Each DCPAM ETL instance can use dedicated Staging Area node (local or remote).
* Many DCPAM Database nodes can be encapsulated into single data access point by DCPAM WDS.
* DCPAM ETL, Staging Area, DCPAM Database and DCPAM WDS can run on single server as well!

Following diagrams represents different configurations to deploy both Data Warehouse and Data Marts.

![DCPAM overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/dcpam-dws.png)
*<p align=center>Data Warehouse with DCPAM</p>*

![DCPAM overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/dcpam-dm.png)
*<p align=center>Data Marts with DCPAM Data Warehouse</p>*

#### DCPAM LCS - Live Component State
[DCPAM LCS](https://github.com/OrionExplorer/dcpam/tree/master/src/DCPAM_LCS/README.md) is the central repository of informations about all the DCPAM Components current state. It is the most important DCPAM Component from an administrative point of view.

#### DCPAM Construct
DCPAM Construct is going to be the main system administration web application.

#### DCPAM Access
DCPAM Access is going to be dedicated Bussiness Inteligence web application.

#### DCPAM Monitoring
DCPAM Access is going to be web application for DCPAM LCS data presentation.

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

> \* SQL Server/Azure SQL Database: [ODBC is the primary native data access API for applications written in C and C++ for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server).

**Please note that DCPAM provides support for every ODBC-compliant data source**.

#### Flat Files
| Data source                        | Support          |
|:-----------------------------------|:----------------:|
| CSV                                | native           |
| TSV                                | native           |
| PSV                                | native           |
| JSON                               | native           |
| XLS                                | [script](https://github.com/OrionExplorer/dcpam/src/DCPAM_ETL/data-processor/excel2csv.py)
| XLSX                               | script           |
| XLSM                               | script           |
| XLSB                               | script           |
| ODF                                | script           |
| ODS                                | script           |

DCPAM can access files from local or remote locations. The latter are fetched via HTTP/HTTPS protocol and [Battery HTTP Server](https://github.com/OrionExplorer/battery-http-server) is recommended.
Files are loaded to temporary tables in DCPAM or external database to make SQL operations possible for this kind of data.

### DCPAM Database
DCPAM is designed to be as most customizable as it needs to be.
Therefore every database listed above as available data source can also be used as DCPAM target database.

### Docker images
Each DCPAM Component is provided with `Dockerfile` to build Docker image:
* DCPAM ETL: `dcpam-etl.dockerfile`
* DCPAM RDP: `dcpam-rdp.dockerfile`
* DCPAM WDS: `dcpam-wds.dockerfile`
* DCPAM LCS: `dcpam-lcs.dockerfile`

##### Currently under active development
* [ ] To be announced.

## Roadmap
| Year | Quarter | Feature                                      | Status            |
|:----:|:-------:|:---------------------------------------------|:------------------|
| 2020 | Q3      | :white_check_mark: Transform process         | Done (2020-08-07) |
| 2020 | Q3      | :white_check_mark: DCPAM WDS                 | Done (2020-08-17) |
| 2020 | Q3      | :white_check_mark: DCPAM auth keys           | Done (2020-08-19) |
| 2020 | Q3      | :white_check_mark: Data source: CSV/TSV/PSV  | Done (2020-08-30) |
| 2020 | Q3      | :white_check_mark: Remote files              | Done (2020-09-01) |
| 2020 | Q3      | :white_check_mark: Data source: JSON         | Done (2020-09-03) |
| 2020 | Q3      | :white_check_mark: DCPAM LCS                 | Done (2020-09-15) |
| 2020 | Q3      | :white_check_mark: Docker images for all the DCPAM Components   | Done (2020-09-20)   |
| 2020 | Q3      | :white_check_mark: HTTPS support             | Done (2020-09-22) |
| 2020 | Q3      | :white_check_mark: Data source: externally preprocessed         | Done (2020-09-24)   |
| 2020 | Q4      | DCPAM ETL DB connectivity tests              | Not implemented   |
| 2020 | Q4      | Data source: APIs                            | Not implemented   |
| 2021 | Q1      | DCPAM Monitoring                             | Not implemented   |
| 2021 | Q2      | DCPAM Access (BI web application)            | Not implemented   |
| 2021 | Q2      | Data source: XML                             | Not implemented   |
| 2021 | Q3      | DCPAM Construct (admin web application)      | Not implemented   |
| 2021 | Q4      | DCPAM AI Platform                            | Not implemented   |

---
This software uses:
* [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.
* [SQLite3](https://www.sqlite.org/ "SQLite")
* [pandas](https://pandas.pydata.org/ "pandas") for data processing

[1]: https://en.wikipedia.org/wiki/Snowflake_schema
[2]: https://en.wikipedia.org/wiki/Star_schema
[3]: https://en.wikipedia.org/wiki/Fact_constellation
