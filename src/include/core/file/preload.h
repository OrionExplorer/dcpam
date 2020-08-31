#ifndef PRELOAD_H
#define PRELOAD_H

#include "../../utils/log.h"
#include "../../core/db/system_schema.h"


int FILE_ETL_preload( DATABASE_SYSTEM* cfg, const char* filename, LOG_OBJECT* log );

#endif
