#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utillib.h"
#include "command.h"
#include "parser.h"

/* At the beginning. */
#define CMD_COUNT 1

command_t* parse_cmd( char* line ) {
   /* Shell now allows only one program at the time.*/
   command_t* cmd = ALLOCATE(command_t, CMD_COUNT);
   size_t count, i = 0; 
   char** tmp = split( line, ' ', &count );
   if(tmp == NULL ) return NULL;
   cmd->util = strdup(tmp[0]);
   cmd->args_count = count;
   cmd->args = ALLOCATE( char*, count+1);
   for(i = 0; i < count; i++ ) 
      cmd->args[i] = strdup(tmp[i]);
   cmd->args[count] = NULL;

   if(tmp) { 
      for(i = 0; i < count; i++)
         if(tmp[i])free(tmp[i]);
      free(tmp);
   }
   return cmd;
}
error_t execute( command_t* cmd, int* stat ) {

   pid_t pid;
   int status;

   pid = vfork();
   if( pid == 0 ) {
      execvp( cmd->util, cmd->args);
      _exit(EXIT_FAILURE);
   }
   else if( pid < 0 ) {
      return errno; 
   }
   else {
      if( waitpid( pid, &status, 0 ) != pid ) 
         return errno;
      *stat = WEXITSTATUS(status);
   }
   return OK;
}
