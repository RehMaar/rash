#ifndef ENVIRON_H
#define ENVIRON_H

#include "utillib.h"
#include "map_string.h"

INIT_MAP_STRUCT( env )

INIT_MAP_FUNC_HEADER( env )

error_t set_env( char** pathnames );

#endif /* ENVIRON_H */
