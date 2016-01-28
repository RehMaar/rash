#include <stdio.h>
#include <readline/readline.h>

#include "environ.h"
#include "config.h"
#include "command.h"
#include "utillib.h"
#include "lexer.h"
#include "parser.h"
#include "sig_manager.h"

#define __GNU_READLINE 0


#define MAX_CONFIG_COUNT 1


int main( int argc, char **argv ) {

   char* line = NULL;
#if __GNU_READLINE != 1
   size_t len = 0;
#endif
   int state; 

  (void)set_signal();

   const char* confignames[MAX_CONFIG_COUNT+1] = { 
      "$HOME/rashrc", NULL
   };

   (void)run_init_script( confignames );  
   (void)set_default_environ();

#if __GNU_READLINE == 1
   while(( line = readline( getenv("PROMPT") ))) {
#else
   
   while((fputs(getenv("PROMPT"), stdout))
            && getline( &line, &len, stdin) != -1 ) {
#endif
      command_t* head = NULL; 
      char** tokens = NULL;

      if( line == NULL ) break;
      if( is_empty( line ) == true) continue;
#if __GNU_READLINE != 1
      if(line[strlen(line)-1] == '\n') line[strlen(line)-1]=0;
#endif

      if((tokens = splittok( (const char*)line )) == NULL ) {
         fputs( "Error while parsing command.\n", stderr );
         continue;
      }

      if((state = parse_cmd( tokens, &head ))) {
         (void)set_exit_status( state );
         (void)destroy_tokens( tokens );
         (void)destroy_cmd_list( head ); 
         fputs( "Error while parsing command.\n", stderr );  
         continue;
      }
#if DEBUG == 1
      //SHOW(tokens)
      (void)print_commands( head );
#endif
      state = execute( head );
      if( state == ENOENT ) 
         fprintf(stderr, "%s: command not found `%s'", argv[0],head->name);
      (void)set_exit_status( state );
      (void)destroy_tokens( tokens );
      (void)destroy_cmd_list( head ); 
      head = NULL;
   }
   if(line) free(line);
   (void)destroy_shell_var();
   return 0;
}
