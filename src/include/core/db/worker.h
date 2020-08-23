#ifndef DB_WORKER_H
#define DB_WORKER_H

#include "../../shared.h"
#include "../../utils/log.h"

int             DB_WORKER_init( LOG_OBJECT* log );
int             DB_WORKER_shutdown( LOG_OBJECT* log );

#endif
