#ifndef CONFIG_H
#define CONFIG_H

#include "utillib.h"

#define MAX_CONFIG 1
error_t run_init_script( const char** confignames );

error_t read_script( const char* confignames );

#endif /* CONFIG_H */
