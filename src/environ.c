#include <string.h>
#include <stdio.h>

#include "environ.h"

INIT_MAP_FUNC( environ ) 

//extern char** environ;
static environ_t* shell_environ;

#define MAX_DEFAULT 1

static struct _default_vars_t {
   char* name;
   char* value;
} default_vars[MAX_DEFAULT] = {
   { "PROMPT", "rash > " }
};

void set_cwd( void ) {
   char* cwd, *old_cwd;
   if((cwd = getcwd(NULL, 0))) {
      if((old_cwd = getenv( "PWD"))){
         setenv( "OLDPWD", old_cwd, 1);
        //free(old_cwd);
      }
      else {
         setenv( "OLDPWD", cwd, 1 );
      }
      setenv( "PWD", cwd, 1 );
      free(cwd);
   }
   else {
      perror( "pwd" );  
   }
}

void set_default_environ( void ) {
   set_cwd();
   for( int i = 0; i < MAX_DEFAULT;i++) {
      setenv( default_vars[i].name, default_vars[i].value, 0);
   }
}

error_t set_shell_var( environ_t* env ) {
   if( env && env->value && env->key ) {
      shell_environ = set_environ_value( shell_environ, env->key, env->value );  
      return SUCCESS;
   }
   return ERROR;
}
char* get_shell_var( const char* key ) {
   return get_environ_value( shell_environ, key );
}

void destroy_shell_var( void ) {
   destroy_environ_map( shell_environ );
} 
