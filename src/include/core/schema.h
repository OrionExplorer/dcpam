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

#ifndef SCHEMA_H
#define SCHEMA_H

#include "db/etl_schema.h"
#include "db/system_schema.h"
#include "app_schema.h"
#include "../shared.h"
#include "../DCPAM_LCS/dcpam-lcs.h"

int                         DATABASE_SYSTEMS_COUNT;
DATABASE_SYSTEM             DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];   /* Database-based systems | config.json => "system" */

DCPAM_APP                   APP;                                    /* Main application object | config.json => "app" */
P_DCPAM_APP                 P_APP;                                  /* Main application object | config.json => "app" */
L_DCPAM_APP                 L_APP;                                  /* Main application object | config.json => "app" */

#endif
