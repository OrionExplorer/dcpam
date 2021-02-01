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

#ifndef CDC_TRANSFORM
#define CDC_TRANSFORM

#include "../etl_schema.h"
#include "../system_schema.h"

int DB_CDC_TransformInserted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT* log );
int DB_CDC_TransformDeleted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT* log );
int DB_CDC_TransformModified( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT* log );

#endif
