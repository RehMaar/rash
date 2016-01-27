#ifndef ENVIRON_H
#define ENVIRON_H

#include "utillib.h"
#include "map_string.h"

INIT_MAP_STRUCT( environ )

INIT_MAP_FUNC_HEADER( environ )

void destroy_shell_var( void );

void set_cwd( void );
void set_exit_status( int state );
void set_default_environ( void );
void print_shell_var( void );
int set_shell_var( environ_t* env );
int set_shell_var_key( char* key, char* value);
char* get_shell_var( const char* key );

#endif /* ENVIRON_H */
