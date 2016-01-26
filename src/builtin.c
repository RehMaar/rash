#include "utillib.h"
#include "builtin.h"
#include "environ.h"
#include "lexer.h"
#include "config.h"

#include <stdlib.h>
#include <stdio.h>

static error_t export( char** );
static error_t exec( char** );
static error_t cd( char** );
static error_t pwd( char** );

#define BUILTINS_COUNT 4

typedef error_t(*builtin_func_t ) ( char** );
struct _builins_t {
   char* name;
   builtin_func_t func;
} builtins_list[ BUILTINS_COUNT] = {
   { "export", export },
   { "cd",     cd     },
   { "pwd",    pwd    },
   { "exec",   exec   }
};


error_t exec_builtin( char* name, char** args ) {
   error_t state;
   for(int i = 0; i < BUILTINS_COUNT; i++) {
      if( strcmp( builtins_list[i].name, name ) == 0){
         state = builtins_list[i].func( args );
         return state;
      }
   }
   return ENOEXT; 
}

static error_t pwd( char** args ) {
   char* cwd = getenv("PWD");
   if( cwd ) {
      puts( cwd );
      return SUCCESS;
   }
   else {
      perror("pwd");
      return ERROR;
   }
}

static error_t cd( char** args ) {
   return 0;   
}

static error_t exec( char** args ) {
   return  read_script((const char*)args[1]); 
}

static error_t export( char** args ) {
   error_t state = -1;
   int i = 1;
   while( args[i] ) {
      if( strlen(args[i-1]) == 1 && args[i-1][0] == '=' && i != 2 ) {
         state = setenv(args[i-2], args[i], 1 );     
      }
      i++;
   }
   return state;
} 
