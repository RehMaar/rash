#ifndef CONFIG_H
#define CONFIG_H

#include "utillib.h"

#define MAX_CONFIG 1
int sub_path_env( char** str );
int run_init_script( const char** confignames );

int read_script( const char* confignames );

#endif /* CONFIG_H */
