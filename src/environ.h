#ifndef ENVIRON_H
#define ENVIRON_H

#include "utillib.h"

typedef struct {
	char** shell_env;
	char** env; 
} environ_t;

error_t set_env( const char** rc_paths );

#endif /* ENVIRON_H */
