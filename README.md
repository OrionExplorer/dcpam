# DCPAM [![Build Status](https://travis-ci.org/OrionExplorer/dcpam.svg?branch=master)](https://travis-ci.org/OrionExplorer/dcpam)
### _Data Construct-Populate-Access-Manage_
* Data warehouse or data mart.
* DCPAM helps to create central repositories of integrated data from one or disparate sources([1]).
* Cross-platform (Linux/Windows).

It is ETL-based([2]) system with CDC([3]) solutions depending on the data sources (currently it's database only):

| CDC solution                            | Source        |
|-----------------------------------------|:-------------:| 
| SQL query for timestamps (eg. MIN, MAX) | Database      |
| SQL query for diffs (eg. IN, NOT IN)    | Database      |


![Architecture overview](https://raw.githubusercontent.com/OrionExplorer/dcpam/master/docs/architecture.png)


### Data sources
DCPAM is still work in progress, with following data sources:
|  ID  | Data source     | Type            | Support | Status      |
|:----:|:--------------  |:---------------:|:-------:|:-----------:|
| 1    | PostgreSQL      | Database        | Native  | Available   |
| 2    | MySQL           | Database        | Native  | Available   |
| 3    | MariaDB         | Database        | Native  | Available   |
| 4    | SQL Server      | Database        | ODBC    | Available   |
| 5    | Oracle Database | Database        | Native  | In progress |


### Configuration
File `config.json` is DCPAM foundation. It defines:
* Data sources
* Extract, Transform and Load process for each data source
* DCPAM database, tabels and views (see _app.DATA_), where integrated data is stored


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
- run with Valgrind (with suppression of Oracle OCI errors) and `config_mysql.json` (where MySQL is DCPAM main database)
```
> valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_oci.supp ./dcpam config_mysql.json
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


This software uses [cJSON](https://github.com/DaveGamble/cJSON "cJSON") for parsing JSON data.

[1]: https://en.wikipedia.org/wiki/Data_warehouse
[2]: https://en.wikipedia.org/wiki/Extract,_transform,_load
[3]: https://en.wikipedia.org/wiki/Change_data_capture
