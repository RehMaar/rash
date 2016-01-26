#include "utillib.h"
#include "builtin.h"
#include "environ.h"
#include "lexer.h"
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int export( char** );
static int exec( char** );
static int cd( char** );
static int pwd( char** );
static int rashenv( char** );

#define BUILTINS_COUNT 5

typedef int(*builtin_func_t ) ( char** );
struct _builins_t {
   char* name;
   builtin_func_t func;
} builtins_list[ BUILTINS_COUNT] = {
   { "export",    export   },
   { "cd",        cd       },
   { "pwd",       pwd      },
   { "rashenv",   rashenv  },
   { "exec",      exec     }
};


int exec_builtin( char* name, char** args ) {
   int state;
   for(int i = 0; i < BUILTINS_COUNT; i++) {
      if( strcmp( builtins_list[i].name, name ) == 0){
         state = builtins_list[i].func( args );
         return state;
      }
   }
   return ENOEXT; 
}

/* rashenv -- args[0], args[1] -- NULL or key */
static int rashenv( char** args ) {
   if( args[1] ) {
      char* value = get_shell_var( (const char*)args[1]);
      if( value )
         printf("%s=%s\n", args[1], value );
      else {
         fprintf( stderr, "No variable: %s.", args[1] );
         return -1;
      }
   } 
   else {
      print_shell_var();
   }
   return 0;
}

static int pwd( char** args ) {
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

static int cd( char** args ) {
   int state;
   char* dir = NULL;
   if( args[1]) {
      if(strcmp(args[1], "-") == 0) {
         if((dir = getenv("OLDPWD")) == NULL) 
            dir = getenv("HOME");
      }
      else 
         dir = args[1];
   }
   state = chdir(dir);
   if( state == -1 )
      perror("cd");
   else {
      set_cwd();
   }
   return 0;   
}

static int exec( char** args ) {
   return  read_script((const char*)args[1]); 
}

static int export( char** args ) {
   int state = -1;
   int i = 1;
   while( args[i] ) {
      if( strlen(args[i-1]) == 1 && args[i-1][0] == '=' && i != 2 ) {
         state = setenv(args[i-2], args[i], 1 );     
      }
      i++;
   }
   return state;
} 
