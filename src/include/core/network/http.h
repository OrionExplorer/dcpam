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

#ifndef HTTP_H
#define HTTP_H

#include "../../utils/log.h"
#include "../network/client.h"


typedef struct {
    NET_CONN    *connection;
    char        *path;
} HTTP_CLIENT;

char* HTTP_CLIENT_get_content( HTTP_CLIENT *client, const char *host, const char *path, const int port, const int secure, HTTP_DATA *http_data, size_t *content_len, LOG_OBJECT *log );

#endif
