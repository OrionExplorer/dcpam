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

#ifndef DCPAM_LCS_H
#define DCPAM_LCS_H

#include "../../include/core/component.h"
/*
    lcs_config.json => app
*/
typedef struct L_DCPAM_APP {
    char* version;
    char* name;
    int                     network_port;
    DCPAM_COMPONENT** COMPONENTS;
    int                     COMPONENTS_len;
    DCPAM_ALLOWED_HOST** ALLOWED_HOSTS_;
    int                     ALLOWED_HOSTS_len;
} L_DCPAM_APP;


L_DCPAM_APP         L_APP;

#endif
