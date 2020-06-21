# TODO

## Architecture
* [ ] Rebuild DCPAM architecture to support Staging Area (extracted data is stored in stage_* table, defined in `config.json`)
	* [x] New data type DB_SYSTEM_CDC_STAGE_QUERY.
	* [x] Rebuild type DB_SYSTEM_CDC_LOAD_QUERY to support fetching data from desired stage_* table and load into target table.
	* [x] Rebuild configuration loading for Stage and Load.
	* [x] Rebuild DB_WORKER to support Staging Area.
	* [ ] Remove processed records from Staging Area after successful Load.
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