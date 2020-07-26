# TODO

## Architecture
* [x] Rebuild DCPAM architecture to support Staging Area (extracted data is stored in stage_* table, defined in `config.json`)
	* [x] New data type DB_SYSTEM_CDC_STAGE_QUERY.
	* [x] Rebuild type DB_SYSTEM_CDC_LOAD_QUERY to support fetching data from desired stage_* table and load into target table.
	* [x] Rebuild configuration loading for Stage and Load.
	* [x] Rebuild DB_WORKER to support Staging Area.
	* [x] Remove processed records from Staging Area after successful Load.
* [x] Check Stage and Load subprocesses corectness action-wise:
	* [x] Special query for Staging Area reset after each process from ETL is completed.
* [x] Rebuild DCPAM architecture to store each fetched record directly into Staging Area. That would completely remove memory overhead during Extract process.
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
* [x] Rebuild DCPAM architecture to load each fetched record directly from Staging Area into Target tables. That would completely remove memory overhead during Load process:
	* [x] Rebuild `CDC_LoadGeneric` internals
	* [x] Prepare callback functions:
		* [x] `_LoadGeneric_callback`
* [x] DCPAM must perform ETL process differently (v2):
	* [x] New workflow:
		* [x] Extract and stage data from all configured queries for given system...
		* [x] ...then transform all staged data...
		* [x] ...and finally load entire dataset for given system to target tables.
	* [x] Update JSON files for each database configuration to meet new requirements.
* [x] DCPAM must perform ETL process differently (v3, overrides v2):
	* [x] New workflow:
		* [x] Extract and stage data from all configured queries for all systems...
		* [x] ...then transform and combine all staged data across all source systems...
		* [x] ...and finally load entire dataset for all systems to the target tables.
* [x] Make Staging Area optional?
	* [x] Rebuild DCPAM config to support lack of Staging Area descriptions.
	* [x] Extract subprocess callback can be either Stage or Load function.
* [ ] Each ETL process must handle unique DCPAM Database connection in order to work properly multithreaded.
	* [ ] Update struct DATABASE_SYSTEM.
	* [ ] Update function DATABASE_SYSTEM_DB_add.
* [ ] Staging Area can be placed in different database.
	* [x] Rebuild DCPAM config.
	* [ ] Rebuild Extract subprocess callback.
	* [ ] Rebuild Load subprocess.
* [ ] Transform subprocess (https://en.wikipedia.org/wiki/Extract,_transform,_load#Transform):
	* [ ] Proposal #1: simple internal operations within dcpam (NO-GO: hard-coded rules, small range of usable functions).
	* [x] Proposal #2: make use of external scripts/applications.
	* [ ] Proposal #3: LUA scripts (NO-GO: DCPAM engine would require major rebuild and new dependencies, also speed is concern).
	* [x] Thought #1: Do we need to force the usage of Transform subprocess? During Extraction we can store all necessary data in the Staging Area. Then, with Load subprocess, combine the data to the desired form in the target tables by using SQL.
		* [x] Rebuild DCPAM config to support lack of Transform descriptions.
	* [x] Thought #2: Transformation can be performed on different server:
		* Scripts/applications for transformation are placed here.
		* New DCPAM tool for communication with DCPAM engine.
		* Possible to call Transformation scripts/applications locally and remotely.
	* [ ] Implementation:
		* [x] Rebuild DCPAM config.
		* [ ] Run external script/application (`popen` for local and sockets for remote).
		* [ ] Provide all the necessary auth data to external script/application with command line arguments. 
		* [ ] Wait for external script/application to finish with `EXIT_SUCCESS`.
		* [ ] Proof of concept script:
			* [ ] Python script.
			* [ ] Remove leading and trailing spaces.
			* [ ] Backflow of cleaned data in the original source.
		* [ ] DCPAM EXEC:
			* [ ] Socket server application capable to deal with many connected clients.
			* [ ] Use of `popen` to call external script/application.
* [ ] ETL process interval can vary between system[].queries[].
* [ ] Move DCPAM configuration to database:
	* [ ] Table schemas based on `config.json`.
	* [ ] Schema creation during DCPAM installation.
	* [ ] `config.json` is still valid first-class citizen.


## Data sources
* [ ] New data sources:
	* [x] MariaDB
	* [x] Oracle Database
	* [x] SQLite
	* [ ] MongoDB
	* [ ] CSV
	* [ ] URL
	* [ ] JSON
	* [ ] LDAP