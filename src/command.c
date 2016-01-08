#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "command.h"

static size_t count_tokens( char* str, const char delim ) {
   size_t count = 0; 
   char last_char = ' ';
   for( size_t i = 0; str[i]; i++ ) {
      if( str[i] == delim && last_char != delim) {
         count++;
      }
      last_char = str[i]; 
   }
   if(count != 0 || last_char != delim) {
      count++;
      count += last_char == delim ? -1: 0;
   }
   return count; 
}

static char** split( char* line, const char delim, size_t* count) {
   size_t c = count_tokens( line, delim );
   char* tmp, *save;
   char** tokens; 
   char delims[2]; delims[0] = delim; delims[1] = 0;
   size_t i = 0;
   *count = c;
   if(c == 0 ) return NULL;

   tokens = ALLOCATE( char*, c);
   tmp = strtok_r( line, delims, &save);
   while(tmp != NULL ) {
      tokens[i++] = strdup(tmp);
      tmp = strtok_r(NULL, delims, &save);
   }

   return tokens;
}

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
   for(i = 0; i < count; i++ ) {
      cmd->args[i] = strdup(tmp[i]);
      cmd->args[count] = NULL;
   }
   if(tmp != NULL ) { 
      for(i = 0; i < count; i++)
         if(tmp[i] != NULL )free(tmp[i]);
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
