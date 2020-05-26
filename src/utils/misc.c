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
