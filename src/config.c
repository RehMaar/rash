#include "environ.h"
#include "config.h"
#include "parser.h"
#include "command.h"
#include "lexer.h"
#include <ctype.h>
#include <errno.h>


/* TODO: substitute all environments. */
int sub_path_env( char** str ) {
   char* tmp, *token, *value, *save;
   
   if( !strchr( *str, DOLLAR ) ) return SUCCESS;

   token = strtok_r( *str, "$/", &save);
   if((value = getenv(token)) == NULL )
      return ENOMATCH;  
   tmp = ALLOCATE(char, (strlen(value)+strlen(save)+2)); // / + \0  
   sprintf(tmp,"%s/%s",value,save);
   free(*str);
   *str = tmp;

   return SUCCESS;
}

int run_init_script( const char** confignames ) {
   int i = 0;
   while( confignames[i]) {
      char* config = strdup( confignames[i++]);
      sub_path_env( &config );
      (void)read_script(config);
      free(config);
   }
   return 0;
}

#define MAX_LINE_LENGTH 256

int read_script( const char* configname ) {

   FILE* fs; 
   char* line = NULL;
   size_t len = 0;
   int state; 
   command_t* head = NULL;
   char** tokens = NULL;
   ssize_t read;

   if(access(configname, R_OK) != 0 ) {
#ifndef DEBUG
      perror( configname );
#endif
      return ENOVALIDRC;
   }
   if(( fs = fopen( configname, "r" ) ) == NULL ) {
#ifndef DEBUG
      perror( configname );
#endif
      return ENOVALIDRC; 
   }
   while( (read = getline( &line, &len, fs )) != -1 ) {
      if( line[0] == '\n' || line[0] == '#' || (is_empty(line) == true) ) 
         continue;
      if((len = strlen(line)) && line[len-1] == '\n') 
         line[len-1] = '\0';

      if(( tokens = splittok((const char*)line) ) == NULL ) 
         continue; 
      if(( state = parse_cmd( tokens, &head ))) {
         (void)fprintf(stderr, "Error: %d\n", state); 
         (void)destroy_tokens( tokens );
         continue;
      }
#ifdef DEBUG
      (void)print_commands( head );
#endif
      (void)destroy_tokens( tokens );
      if(( state = execute( head ) )) {
         (void)fprintf(stderr, "Error: %d\n", state); 
         continue;
      }
      if( head == NULL ) 
         (void)fprintf( stderr, "Error: command parsing\n");
      else {
         (void)destroy_cmd_list( head ); 
         head = NULL;
      }
      
   }
   (void)set_exit_status(state);
   if( line ) free( line );
   if( head ) destroy_cmd_list( head ); 
   if( fs ) fclose( fs );
   return state;
}
