#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "environ.h"
#include "config.h"
#include "command.h"
#include "utillib.h"
#include "lexer.h"
#include "parser.h"
#include "sig_manager.h"

#define __GNU_READLINE 

#define MAX_CONFIG_COUNT 1

static const char* confignames[MAX_CONFIG_COUNT+1] = { 
      "$HOME/.rashrc",
      NULL
};


int main( int argc, char **argv ) {

#ifdef __GNU_READLINE
  const char* _history_path = "$HOME/.rash_history";
  char* history_path = strdup( _history_path );
#endif
   char* line = NULL;
#ifndef __GNU_READLINE 
   size_t len = 0;
#endif
   int state; 

#ifdef __GNU_READLINE
   (void)sub_path_env( &history_path );
   read_history( history_path );  
#endif
   (void)set_signal();

   
   (void)run_init_script( confignames );  
   (void)set_default_environ();

#ifdef __GNU_READLINE 
   while(( line = readline( getenv("PROMPT") ))) {
#else
   
   while((fputs(getenv("PROMPT"), stdout))
            && getline( &line, &len, stdin) != -1 ) {
#endif
      command_t* head = NULL; 
      char** tokens = NULL;

      if( line == NULL ) break;
      if( is_empty( line ) == true) continue;
#ifdef __GNU_READLINE
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
#ifdef DEBUG
      (void)print_commands( head );
#endif
      state = execute( head );
      if( state == ENOEXT ) 
         fprintf(stderr, "%s: command not found `%s'\n", argv[0],head->name);
#ifdef __GNU_READLINE 
      add_history( line );
#endif
      (void)set_exit_status( state );
      (void)destroy_tokens( tokens );
      (void)destroy_cmd_list( head ); 
      head = NULL;
   }
#ifdef __GNU_READLINE
   if( history_path ) {
      write_history( history_path );
      free( history_path );
   }
#endif
   if(line) free(line);
   (void)destroy_shell_var();
   return 0;
}
