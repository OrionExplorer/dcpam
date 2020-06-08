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
| 1    | PostgreSQL      | Database        | Native  | Done        |
| 2    | MySQL           | Database        | Native  | Done        |
| 3    | MariaDB         | Database        | Native  | Done        |
| 4    | SQL Server      | Database        | ODBC    | Done        |
| 5    | Oracle Database | Database        | Native  | Research    |


### Configuration
File `config.json` is DCPAM foundation. It defines:
* Data sources
* Extract, Transform and Load process for each data source
* DCPAM database, tabels and views (see _app.DATA_), where integrated data is stored


#### Compilation
##### Dependencies
- libpq-dev (PostgreSQL)
- libmysqlclient-dev (MySQL)
- libmariadbclient-dev (MariaDB)
- unixodbc-dev (ODBC)
- [cJSON](https://github.com/DaveGamble/cJSON "cJSON") (included)

```
> make
```
- run
```
> ./dcpam
```
- run with Valgrind
```
> valgrind --tool=memcheck --leak-check=yes ./dcpam
```

[1]: https://en.wikipedia.org/wiki/Data_warehouse
[2]: https://en.wikipedia.org/wiki/Extract,_transform,_load
[3]: https://en.wikipedia.org/wiki/Change_data_capture
