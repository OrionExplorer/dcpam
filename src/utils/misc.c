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

#include "../include/shared.h"
#include "../include/utils/misc.h"
#include <stdio.h>
#include <stdlib.h>


int ARRAY_has_int_element( int *src_array, int array_size, int element ) {
    int         i = 0;
    int         result = -1;

    for( i = 0; i < array_size; i++ ) {
        if( src_array[i] == element ) {
            result = i;
            break;
        }
    }

    return result;
}
