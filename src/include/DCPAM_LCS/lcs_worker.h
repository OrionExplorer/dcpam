#ifndef LCS_WORKER_H
#define LCS_WORKER_H

#include "../utils/log.h"

int LCS_WORKER_shutdown( LOG_OBJECT* log );
void *LCS_WORKER_watcher( void *p );
#endif
