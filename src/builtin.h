#ifndef BUILTIN_H
#define BUILTIN_H

#include "utillib.h"

int execute_builtin( int index, char** args );
int is_builtin( char* name );

#endif /* BUILTIN_H */
