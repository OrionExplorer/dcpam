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

#ifndef CDC_LOAD
#define CDC_LOAD

#include "../etl_schema.h"
#include "../system_schema.h"

void _LoadGeneric_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _LoadInserted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _LoadDeleted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _LoadModified_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
int DB_CDC_LoadInserted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT* log );
int DB_CDC_LoadDeleted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT* log );
int DB_CDC_LoadModified( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT* log );

#endif
