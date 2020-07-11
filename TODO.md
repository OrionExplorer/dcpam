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
* [x] DCPAM must perform ETL process differently:
	* [x] New workflow:
		* [x] Extract and stage data from all configured queries for given system...
		* [x] ...then transform all staged data...
		* [x] ...and finally load entire dataset for given system to target tables.
	* [x] Update JSON files for each database configuration to meet new requirements.
* [ ] Transform subprocess (https://en.wikipedia.org/wiki/Extract,_transform,_load#Transform):
	* [ ] Proposal #1: simple internal operations within dcpam.
	* [x] Proposal #2: make use of external scripts/applications.
	* [ ] Proposal #3: LUA scripts.
	* [ ] Implementation
* [ ] ETL process interval can vary between system[].queries[].

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