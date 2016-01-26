#include <stdio.h>
#include <readline/readline.h>

#include "environ.h"
#include "config.h"
#include "command.h"
#include "utillib.h"
#include "lexer.h"
#include "parser.h"


#define MAX_CONFIG_COUNT 1
#define DEBUG

int main( int argc, char **argv ) {

   char* line = NULL;
   error_t state; 
   int stat;
  
   const char* confignames[MAX_CONFIG_COUNT+1] = { 
      "$HOME/rashrc", NULL
   };

   run_init_script( confignames );  
   set_default_environ();

   while(( line = readline( getenv("PROMPT") ))) {
      command_t* head = NULL; 
      char** tokens = NULL;

      if( line == NULL ) break;
      if( is_empty( line ) == true) continue;

      if((tokens = splittok( (const char*)line )) == NULL ) {
         FREE_MATRIX(tokens)
         fputs( "Error while parsing command.\n", stderr );
         continue;
      }
      
      if((state = parse_cmd( tokens, &head ))) {
         FREE_MATRIX(tokens)
         cmd_list_free( head ); 
         fprintf( stderr, "Error while parsing command: %d\n", state);  
         continue;
      }
#ifdef DEBUG
      print_commands( head );
#endif
      if(( state = execute( head, &stat )) == SUCCESS ) { 
         if( stat != 0 )
            fprintf( stderr,"Error:%d\n", stat);
      }
      else {
         fprintf( stderr,"Error: %s\n", strerror(state));     
      }
      FREE_MATRIX(tokens)
      cmd_list_free( head ); 
      head = NULL;
   }
   if(line) free(line);
   destroy_shell_var();
   return 0;
}
