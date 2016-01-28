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
*/


typedef enum {
  STDIN = 0, 
  STDOUT,
  STDERR
} stdfd_t;

typedef enum {
   FILENAME = 0,
   FDR,
   FDW
} redir_type_t;

typedef struct redir_map_t {
   union {
      char* name;
      stdfd_t fd;
   } source;
   union {
      char* name;
      stdfd_t fd; 
   } target;
   redir_type_t type_source;
   redir_type_t type_target;
   struct redir_map_t* next;
} redir_map_t;

typedef enum {
   SIMPLE = 0,
   ENVIRON = 1,
   PIPELINE
} cmd_type_t;

typedef struct command_t {
   environ_t* env; 
   redir_map_t* redir_map;
   char* name;
   char** args;
   
   cmd_type_t type; 

   struct command_t* next;
//   struct command_t* group; /* */
} command_t;


void cmd_list_add_back( command_t** list, command_t* tmplist );
void destroy_cmd_list(command_t *list);

void redir_map_add_back( redir_map_t** list, redir_map_t* tmplist );
void destroy_redir_map( redir_map_t *list);


#ifdef DEBUG
   void print_commands( const command_t* head );
#endif

int execute( command_t* head );

#endif /* COMMAND_H */
