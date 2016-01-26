#include "environ.h"
#include "config.h"
#include "parser.h"
#include "command.h"
#include "lexer.h"

#include <errno.h>


static int sub_env( char** str ) {
   char* tmp, *token, *value, *save;
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
      sub_env( &config );
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
   int stat; 
   int state; 
   command_t* head = NULL;
   char** tokens = NULL;

   if(access(configname, R_OK) != 0 ) {
      perror( configname );
      return errno;
   }
   if(( fs = fopen( configname, "r" ) ) == NULL ) {
      perror( configname );
      return errno; 
   }
   ssize_t read;
   while( (read = getline( &line, &len, fs )) != -1 ) {
      if( line[0] == '\n' || line[0] == '#' || (is_empty(line) == true) ) 
         continue;
      if((len = strlen(line)) && line[len-1] == '\n') 
         line[len-1] = '\0';

      if(( tokens = splittok((const char*)line) ) == NULL ) 
         continue; 
      if(( state = parse_cmd( tokens, &head ))) {
         fprintf(stderr, "Error: %d\n", state); 
         destroy_tokens( tokens );
         continue;
      }
      destroy_tokens( tokens );
      if(( state = execute( head ) )) {
         fprintf(stderr, "Error: %d\n", state); 
         continue;
      }

      if( head == NULL ) 
         fprintf( stderr, "Error: command parsing\n");
      else {
         cmd_list_free( head ); 
         head = NULL;
      }
   }

   if( line ) free( line );
   cmd_list_free( head ); 
   if( fs ) fclose( fs );

   return SUCCESS;
}
