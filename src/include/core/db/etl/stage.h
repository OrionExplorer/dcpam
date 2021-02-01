/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

#ifndef CDC_STAGE
#define CDC_STAGE

#include "../etl_schema.h"
#include "../system_schema.h"

/*
	We reuse DB_SYSTEM_ETL_STAGE because technically both processes are the same.
*/
void DB_CDC_StageGeneric( DB_SYSTEM_ETL_STAGE* stage, DB_SYSTEM_ETL_STAGE_QUERY* stage_element, DATABASE_SYSTEM_DB* db, DB_RECORD* record, LOG_OBJECT* log );
void DB_CDC_StageInserted( DB_SYSTEM_ETL_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD *record, LOG_OBJECT* log );
void DB_CDC_StageDeleted( DB_SYSTEM_ETL_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD* record, LOG_OBJECT* log );
void DB_CDC_StageModified( DB_SYSTEM_ETL_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD* record, LOG_OBJECT* log );

#endif
