#ifndef COMMANDS_H
#define COMMANDS_H

#include "utillib.h"

typedef struct {
   char* util; /* All programms in command.*/
   char** args; /* Arguments located in the progs order respectively.*/
   size_t args_count; 
} command_t; 

#define FREE_COMMAND( cmd )                  \
   if(cmd != NULL ) {                        \
      if(cmd->util != NULL ) free(cmd->util);\
      if(cmd->args != NULL ) free(cmd->args);\
      free(cmd);                             \
   }  


command_t* parse_cmd( char* line );

error_t execute( command_t* cmd, int *stat );

#endif /* COMMANDS_H */
