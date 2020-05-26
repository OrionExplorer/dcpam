#ifndef DB_SYSTEM_H
#define DB_SYSTEM_H

#include "../../shared.h"
#include "../../core/schema.h"

void DATABASE_SYSTEM_add(
            const char              *name,
            DATABASE_SYSTEM_DB      *db,
            DATABASE_SYSTEM_QUERY   queries[ MAX_SYSTEM_QUERIES ],
            const int               queries_len,
            short                   verbose
);


void DATABASE_SYSTEM_DB_add(
            const char              *ip,
            const int               port,
            const DB_DRIVER         driver,
            const char              *user,
            const char              *password,
            const char              *db,
            DATABASE_SYSTEM_DB      *dst,
            short                   verbose
);


int DATABASE_SYSTEM_DB_init (
            DATABASE_SYSTEM_DB      *db
);


void DATABASE_SYSTEM_close(
            DATABASE_SYSTEM         *system
);


void DATABASE_SYSTEM_QUERY_add(
            const char              *name,
            DB_SYSTEM_CDC           cdc,
            const char              data_types[ SMALL_BUFF_SIZE ][ SMALL_BUFF_SIZE ],
            const int               data_types_len,
            DATABASE_SYSTEM_QUERY   *dst,
            short                   verbose
);

int DB_exec(
            DATABASE_SYSTEM_DB      *db,
            const char              *sql,
            unsigned long           sql_length,
            DB_QUERY                *dst_result,
            const char* const       *param_values,
            const int               params_count,
            const int               *param_lengths,
            const int               *param_formats,
            const char              *param_types
);

#endif
