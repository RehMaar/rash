#ifndef COMMAND_H
#define COMMAND_H

#include "utillib.h"
#include "environ.h"
/*
   Metachar: ; < > | $ = 
   
   Allowed commands: 
      list         --  cmd; cmd; cmd
      pipes        --  cmd | cmd | cmd
      redirections --  cmd > file
      environs     --  name=value; name=value cmd
   Sample: 
      LANG=ru_RU.UTF-8 ls -l; TEST=test   
      Tokens:
         LANG
         =
         ru_RU.UTF-8
         ls
         -l
         ;
         TEST
         =
         test
*/

enum {
   SIMPLE,
   ENVIRON,
   REDIRECTED,
   PIPE_IN,
   PIPE_OUT,
   PIPE
} cmd_type_t;


typedef struct command_t {
   environ_t* env; 
   char* name;
   char** args;
   
   struct command_t* next;
   //struct command_t* group; /* */
} command_t;

/* Add a node to the end of list from the local atack var. */
void cmd_list_add_back( command_t** list, command_t* tmplist );
/* Free allocated space. */
void cmd_list_free( command_t * list);
void print_commands( const command_t* head );

error_t execute( command_t* head, int *stat );
#endif /* COMMAND_H */
