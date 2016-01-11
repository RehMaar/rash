#include <string.h>

#include "environ.h"
#include "rc_parser.h"

//#include "map_string.h"

INIT_MAP_FUNC( env ) 

extern char** environ;


/* Leaks, my leaks! */
error_t substitute_env( char** str ) {
   char* tmp, *ttmp, *token, *value, *save;
   tmp = strdup(*str);
   token = strtok_r( tmp, "$/", &save);
   if((value = getenv(token)) == NULL )
      return ENOMATCH;  
   ttmp = ALLOCATE(char, (strlen(value)+strlen(save)+2)); // / + \0  
   strcpy(ttmp, value);
   strcat(ttmp, "/");
   strcat(ttmp, save);
   *str = ttmp;
   
   free(tmp);
   return OK;
} 

error_t set_env( char** pathnames ) {
  error_t state; 
  int i = 0;
  env_t* env_map = NULL; 

  while( pathnames[i] != NULL) substitute_env(&pathnames[i++]); 
  if((state = rc_parser( pathnames, &env_map)) != OK )
      return state;

  env_t* tmp_env = env_map; 
  while( tmp_env ) {
   setenv( tmp_env->key, tmp_env->value, 1);
   tmp_env = tmp_env->next;
  }
  return OK;
}
