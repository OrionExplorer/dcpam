# TODO

## Architecture
* [x] Rebuild DCPAM ETL architecture to support Staging Area (extracted data is stored in stage_* table, defined in `config.json`)
	* [x] New data type DB_SYSTEM_CDC_STAGE_QUERY.
	* [x] Rebuild type DB_SYSTEM_CDC_LOAD_QUERY to support fetching data from desired stage_* table and load into target table.
	* [x] Rebuild configuration loading for Stage and Load.
	* [x] Rebuild DB_WORKER to support Staging Area.
	* [x] Remove processed records from Staging Area after successful Load.
* [x] Check Stage and Load subprocesses corectness action-wise:
	* [x] Special query for Staging Area reset after each process from ETL is completed.
* [x] Rebuild DCPAM ETL architecture to store each fetched record directly into Staging Area. That would completely remove memory overhead during Extract process.
	* [x] `DB_RECORD` struct must hold field count.
	* [x] `DB_RECORD` would be main callback parameter.
	* [x] `DB_exec` must support callback function called with each fetched row of data.
	* [x] Prepare callback functions:
		* [x] `_ExtractInserted_callback`
		* [x] `_ExtractDeleted_callback`
		* [x] `_ExtractModified_callback`
		* [x] `_ExtractGeneric_callback`
	* [x] Each database `*_exec` function must support callback function passed by `DB_exec`:
		* [x] PostgreSQL
		* [x] MySQL
		* [x] MariaDB
		* [x] ODBC
		* [x] Oracle Database
		* [x] SQLite
	* [x] `CDC_Extract*`  must support callback function to call `CDC_Stage*`.
* [x] Rebuild DCPAM ETL architecture to load each fetched record directly from Staging Area into Target tables. That would completely remove memory overhead during Load process:
	* [x] Rebuild `CDC_LoadGeneric` internals
	* [x] Prepare callback functions:
		* [x] `_LoadGeneric_callback`
* [x] DCPAM ETL must perform differently (v2):
	* [x] New workflow:
		* [x] Extract and stage data from all configured queries for given system...
		* [x] ...then transform all staged data...
		* [x] ...and finally load entire dataset for given system to target tables.
	* [x] Update JSON files for each database configuration to meet new requirements.
* [x] DCPAM ETL must perform differently (v3, overrides v2):
	* [x] New workflow:
		* [x] Extract and stage data from all configured queries for all systems...
		* [x] ...then transform and combine all staged data across all source systems...
		* [x] ...and finally load entire dataset for all systems to the target tables.
* [x] Make Staging Area optional?
	* [x] Rebuild DCPAM ETL config to support lack of Staging Area descriptions.
	* [x] Extract subprocess callback can be either Stage or Load function.
* [x] Each ETL process must handle unique DCPAM Database connection in order to work properly multithreaded.
	* [x] Update struct DATABASE_SYSTEM.
	* [x] Update function DATABASE_SYSTEM_DB_add.
	* [x] Update ETL functions. 
* [x] Staging Area can be placed in external database.
	* [x] Rebuild DCPAM ETL config.
	* [x] Rebuild Extract subprocess callback.
	* [x] Rebuild Load subprocess.
* [x] Pre- and PostETL queries:
	* Would allow to full data extraction and keep previous - now historical - data (by setting record markers or states) in the Warehouse.
	* [x] Implementation:
		* [x] Multiple PreETL statements for each `system.queries[].change_data_capture`.
		* [x] Multiple PostETL statements for each `system.queries[].change_data_capture`.
		* [x] PreETL actions execution.
		* [x] PostETL actions execution.
		* [x] Remove `system.queries[].change_data_capture.stage.reset`.
* [x] Transform subprocess (https://en.wikipedia.org/wiki/Extract,_transform,_load#Transform):
	* [ ] Proposal #1: simple internal operations within dcpam (NO-GO: hard-coded rules, small range of usable functions).
	* [x] Proposal #2: make use of external scripts/applications.
	* [ ] Proposal #3: LUA scripts (NO-GO: DCPAM ETL would require major rebuild and new dependencies, also speed is concern).
	* [x] Thought #1: Do we need to force the usage of Transform subprocess? During Extraction we can store all necessary data in the Staging Area. Then, with Load subprocess, combine the data to the desired form in the target tables by using SQL.
		* [x] Rebuild DCPAM ETL config to support lack of Transform descriptions.
	* [x] Thought #2: Transformation can be performed on different server:
		* Scripts/applications for transformation are placed here.
		* New DCPAM tool for communication with DCPAM engine.
		* Possible to call Transformation scripts/applications locally and remotely.
	* [x] Implementation:
		* [x] Rebuild DCPAM ETL config.
		* [x] Socket-based network layer (client):
			* [x] Protocol.
		* [x] Run external script/application (`popen` for local and sockets for remote).
		* [x] Provide all the necessary auth data to external script/application with command line arguments. 
		* [x] Wait for external script/application to finish with `EXIT_SUCCESS`.
		* [x] Proof of concept script:
			* [x] Python script.
		* [x] DCPAM RDP:
			* [x] Socket server application capable to deal with many connected clients.
			* [x] Use of `popen` to call external script/application.
			* [x] Pass arguments to external script/application.
* [x] Handle ETL errors:
	* [x] For each system separately. Failure on one system does not affect ETL workflow for other systems and does not stop DCPAM ETL service.
* [ ] Example python script for data transformation:
	* [ ] Parse arguments passed by DCPAM RDP.
	* [ ] Connect to data source.
	* [ ] Process staged data.
	* [ ] Perform backflow of cleaned data to source system.
* [x] Allow DCPAM ETL to run once and exit (for scheduled CRON tasks).
* [x] `app.DATA` from `config.json` should be accessed by new subsystem - DCPAM WDS (Warehouse Data Server):
	* DCPAM WDS is dedicated memory caching system for DCPAM BI backend.
	* All the caching is handled by DCPAM WDS.
	* [x] Remove `app.DATA` handlers from DCPAM.
	* [x] Remove `app.DATA` from `config.json`.
	* [x] Create `app.DATA` handlers in DCPAM WDS.
	* [x] Create `app.DATA` in `wds_config.json`.
* [x] DCPAM WDS project and implementation:
	* [x] Support for multiple DCPAM Database nodes.
	* [ ] ~~Load all wds_config\*.json files in application directory.~~
	* [x] Keep list of all DCPAM Database nodes and corresponding predefined queries.
	* [x] Network layer:
		* [x] Based on [c-server-core](https://github.com/OrionExplorer/c-server-core).
		* [x] Protocol:
			* [x] Request: JSON object with 2 parameters:
				* plain SQL query forwarded to RDBMS to deal with
				* DCPAM Database node name
			* [x] Response: JSON array of objects.
		* [x] Cache missing data in real time based on received queries.
	* [x] Memory cache structure.
	* [x] Cache all actions data.
	* [x] DB_CACHE_get function.
* [x] Allowed hosts list for DCPAM components:
	* [x] DCPAM RDP
	* [x] DCPAM WDS
* [x] Client is allowed to connect to each DCPAM component with valid IP address and dedicated API key:
	* [x] DCPAM RDP
	* [x] DCPAM WDS
* [x] DCPAM WDS must response with metadata:
	* [x] success [true|false]
	* [x] data [array]
	* [x] length [number]
* [x] Architecture rebuild for flat files support:
	* [x] New struct: DATABASE_SYSTEM_FLAT_FILE.
	* [x] New DATABASE_SYSTEM element: DATABASE_SYSTEM_FLAT_FILE.
	* [x] Update `etl_config.json` with "FILE" section.
	* [x] Load DATABASE_SYSTEM_FLAT_FILE data from `etl_config.json`.
* [x] Buffered log:
	* Prevent DCPAM components to write the log files with every call of the `LOG_print` function.
	* [x] Define LOG_BUFFER.
	* [x] Rebuild `LOG_print` function.
* [x] Process CSV file data before each ETL start:
	* [x] CSV parser.
	* [x] Load CSV data to desired table.
* [x] Process JSON file data before each ETL start:
	* [x] JSON parser.
	* [x] Load JSON data to desired table.
* [x] Download remote flat files on the fly:
	* [x] Minimal client-side HTTP implementation.
	* [x] Function to download and return a FILE pointer.
* [x] Major DCPAM log rebuild for clean parallel execution and logging:
	* [x] Each processed source has dedicated log file.
	* [x] Init log files during `etl_config.json` load.
	* [x] Rebuild `LOG_*` functions.
	* [x] Modify every source file where logging ocurrs.
* [x] Parameter `max_memory` for DCPAM WDS must be slightly more user-friendly:
	* [x] Value is two-piece string: size and unit (eg. `"max_memory" : "100MB"` which stands for 100 MB of maximum cache size).
	* [x] Supported units are "KB", MB", "GB" and "TB".
* [ ] Automatically refresh cached data in DCPAM WDS:
	* [x] TTL parameter in `wds_config.json`.
	* [x] Expand DCPAM_DATA_ACTION and P_DCPAM_APP.
	* [x] Expand D_CACHE.
	* [ ] Clear and rebuild cache thread.
* [x] DCPAM Warehouse Data Server horizontal scaling:
	* [x] New config: `"WDS"`  array in `wds_config.json`.
	* [x] Search remote nodes for requested table.
	* [x] Every DCPAM WDS node acts as proxy: perform query to remote WDS node, fetch data and response to connected client.
* [x] DCPAM Warehouse Data Server Advanced Query Cache:
	* [x] Cache predefined queries. Queries should not contain any `WHERE`-based conditions.
	* [x] Recognize requested query result (with `WHERE`-based conditions) as a subset of already cached data:
		* [x] Rebuild `CACHE_init` to recognize similar queries.
	* [x] New cache is based on indices to already cached data.
		* [x] New `D_SUB_CACHE` structure to keep indices for `D_CACHE->query->records`.
		* [x] Store `D_CACHE` indices in `D_SUB_CACHE`.
	* [x] Prepare code to handle conditions locally:
		* [x] LIMIT
		* [ ] WHERE with one condition
	* [ ] Switch `row_count` in `DB_QUERY` from `int` to `long`
* [x] DCPAM Components reports to DCPAM LCS:
	* [x] DCPAM ETL
		* [x] Implement LCS_REPORT
		* [x] Report PreETL Actions
		* [x] Report Flat File preload
		* [x] Report Extract process
		* [x] Report data Staging
		* [x] Report Transform process
		* [x] Report Load process
		* [x] Report PostETL Actions
		* [x] Internal ping response
	* [x] DCPAM WDS
		* [x] Implement LCS_REPORT
		* [x] Report every request
		* [x] Report data cache
		* [x] Get current memory usage (cached records) `{"report": "memory", "key" : "zxcasd321"}`
		* [ ] ~~Get connected users~~
		* [x] Internal ping response
	* [x] DCPAM RDP
		* [x] Implement LCS_REPORT
		* [x] Report script execution
		* [x] Internal ping response
* [x] DCPAM LCS reports:
	* [x] Get Components states `{"report": "component_state", "key" : "zxcasd321"}`
	* [x] Get Component actions `{"report": "component_actions", "name" : "DCPAM Remote Data Processor", "key" : "zxcasd321"}`
* [x] Process data fetched from API:
	* [x] Possibility to configure additional HTTP headers (authentication)
	* [ ] ~~Check Content-Type and adjust parsing function (JSON or CSV/TSV/PSV)~~
* [x] Preprocess file formats without native support before DCPAM ETL process starts:
	* [x] Load configuration.
	* [x] Execute processor script/application.
	* [x] Provide Python script to convert XLSX to CSV (using `pandas`).
* [x] Perform connectivity tests before DCPAM ETL starts:
	* [x] `app.DB`
	* [x] `app.STAGING`
	* [x] `system[].DB`
	* [x] `system[].FILE`
* [x] HTTPS support
* [x] Docker images
	* [x] DCPAM ETL
		* [x] Dockerfile with mapping to `etl_config.json`
	* [x] DCPAM RDP
		* [x] Dockerfile with mapping to `rdp_config.json`
	* [x] DCPAM WDS
		* [x] Dockerfile with mapping to `wds_config.json`
	* [x] DCPAM LCS
		* [x] Dockerfile with mapping to `lcs_config.json`
* [x] DCPAM ETL: "extracted", "deleted" and "modified" actions for staging and loading data must support any number of commands.
* [ ] DCPAM ETL: `pre_actions` and `post_actions` to support database definition.
* [ ] DCPAM ETL must not know about P_DCPAM_APP and DPCAM WDS about DCPAM_APP.
* [ ] ETL process interval can vary between system[].queries[].
* [ ] Move DCPAM ETL configuration to database:
	* [ ] Table schemas based on `etl_config.json`.
	* [ ] Schema creation during DCPAM installation.
	* [ ] `etl_config.json` is still valid first-class citizen.


## Data sources
* [ ] New data sources:
	* [x] MariaDB
	* [x] Oracle Database
	* [x] SQLite
	* [x] IBM Db2
	* [ ] MongoDB
	* [ ] Cassandra
	* [x] CSV/TSV/PSV
	* [x] URL
	* [x] JSON
	* [x] XLS/XLSX/XLSM/XLSB
	* [x] ODS/ODT
	* [x] XML
	* [ ] LDAP
	* [x] API
	* [x] Microsoft Access