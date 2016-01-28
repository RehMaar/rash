#include <string.h>
#include <stdio.h>

#include "environ.h"

INIT_MAP_FUNC( environ ) 

static environ_t* shell_environ;

#define MAX_DEFAULT 2

static struct _default_vars_t {
   char* name;
   char* value;
} default_vars[MAX_DEFAULT] = {
   { "EDITOR", "vim"     },
   { "PROMPT", "rash > " }
};

void set_cwd( void ) {
   char* cwd, *old_cwd;
   if((cwd = getcwd(NULL, 0))) {
      if((old_cwd = getenv( "PWD"))){
         setenv( "OLDPWD", old_cwd, 1);
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

void set_exit_status( int state ) {
   char buf[4];
   sprintf( buf, "%d", state);
   shell_environ = set_environ_value( shell_environ, "?", buf);
}
void set_default_environ( void ) {
   char buf[6];
   /* Set PWD and OLDPWD variables. */
   (void)set_cwd();

   /* Set $ -- shell's pid. */
   sprintf( buf, "%d", getpid()); 
   shell_environ = set_environ_value( shell_environ, "$", buf);

   /* Set other shell's variables. */
   for( int i = 0; i < MAX_DEFAULT;i++) {
      setenv( default_vars[i].name, default_vars[i].value, 0);
   }
}

int set_shell_var( environ_t* env ) {
   if( env && env->value && env->key ) {
      shell_environ = set_environ_value( shell_environ, env->key, env->value );  
      return SUCCESS;
   }
   return ERROR;
}
int set_shell_var_key( char* key, char* value) {
   if(value && key ) {
      shell_environ = set_environ_value( shell_environ, key, value );  
      return SUCCESS;
   }
   return ERROR;
 
}
char* get_shell_var( const char* key ) {
   return (char*)get_environ_value( shell_environ, key );
}
void print_shell_var( void ) {
   environ_t* tmp = shell_environ;
   while( tmp ) {
      if( tmp->key && tmp->value )
         printf("%s=%s\n",tmp->key,tmp->value);
      tmp = tmp->next;
   }
}
void destroy_shell_var( void ) {
   (void)destroy_environ_map( shell_environ );
} 
