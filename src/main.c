#include <stdio.h>
#include <readline/readline.h>
#include <time.h>

//#include "builtin.h"
#include "environ.h"
#include "command.h"
#include "utillib.h"

/*
 * 1. Init: env, ID, etc.
 * 2. Read-command loop. 
 * 3. Terminate
 */

#define MAX_CONFIG_COUNT 1

int main( int argc, char **argv ) {
   char* line = NULL;
   command_t* cmd = NULL; 
   error_t state; 
   int stat;
#if DEBUG 
   clock_t start, stop;
#endif
   char* pathnames[MAX_CONFIG_COUNT] = { 
      "$HOME/rashrc/"
   };
   
   //Init shell: read config, etc. 
   if((state = set_env( pathnames ))) 
      fprintf( stderr, "rash: error while setting environment - %d\n", state); 

   while((line = readline( "$ " ))) {
#if DEBUG
      start = clock();
#endif
      cmd = parse_cmd( line ); 
      if( cmd == NULL ) continue;

      state = execute( cmd, &stat ); 
      switch( state ) {
         case OK: 
            switch( stat ) {
               case OK: break;
               default:
                  fprintf( stderr,"%d ", stat);
            }
            break;
         default:
            fprintf( stderr,"%s: %s\n",cmd->util, strerror(state));     
      }
      FREE_COMMAND( cmd ); 
      if(line != NULL ) free(line);      
#if DEBUG
      stop = clock();
      fprintf(stderr,"Time: %f\n",(double)(stop-start)/CLOCKS_PER_SEC);
#endif 
   }
   return 0;
}
