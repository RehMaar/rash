#define __GNU_SOURCE

#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utillib.h"
#include "command.h"
#include "environ.h"
#include "builtin.h"
#include "sig_manager.h"

static command_t *cmd_list_create( command_t* list ){
	command_t *node;
   if( list == NULL ) return NULL;
	node = ALLOCATE(command_t, 1);
   node->env = list->env;
   node->name = list->name;
   node->args = list->args;
   node->type = list->type;
   node->next = NULL;
	return node;		
}

void cmd_list_add_back( command_t **list, command_t* tmplist ){
   command_t *node;
   if(*list == NULL){
      *list = cmd_list_create(tmplist);
      return; 
   }
	node = *list;
	while((*list)->next != NULL)
		*list = (*list)->next;
	(*list)->next = cmd_list_create(tmplist);	
	*list = node;	
}

void cmd_list_free(command_t *list){
	command_t *tail; 
   int i;
	while(list != NULL){
		tail = list->next;
		if( list->env )  destroy_environ_map(list->env);
      if( list->name ) free(list->name);
      if( list->args ) {
         i = 0;
         while( list->args[i]) free(list->args[i++]);
         free( list->args );
      }
      free(list);
		list = tail;
   }
}

#if DEBUG == 1
void print_commands( const command_t* head ) {
   if( head ) {                                                         
      command_t* head_tmp = (command_t*)head;                                     
      while( head_tmp ) {                                             
         puts( "<--- Command --->" );
         printf("> Type: %d\n", head_tmp->type);
         if(head_tmp->name) printf("> Name: %s\n", head_tmp->name);   
         if(head_tmp->args) {                                         
            int head_iter = 0;                                        
            while(head_tmp->args[head_iter]) {                      
               printf("> Args: %s\n", head_tmp->args[head_iter++]);   
            }                                                           
         }                                                              
         if(head_tmp->env) {                                          
           environ_t* head_env = head_tmp->env;                     
           while( head_env ) {                                        
               printf("> Environ: %s=%s\n", head_env->key, head_env->value);  
               head_env = head_env->next;                           
           }                                                            
         }                                                              
         head_tmp = head_tmp->next;                                 
      }                                                                 
   }
}
#endif

static int parent_wait( pid_t child ) {
   int status; 
   int cached_errno;
   waitpid( child, &status, 0 );
   cached_errno = errno;
   if( WIFEXITED( status ) ) {
      return WEXITSTATUS( status );   
   } 
   else if( WIFSIGNALED( status )) {
      fprintf( stderr, "Process terminated with signal %d: %s\n",
               WTERMSIG( status ), strsignal(WTERMSIG(status)));
      if( WIFSTOPPED(status))
         fputs( "Process stoped\n", stderr);   
      else if( WIFCONTINUED(status ))
         fputs( "Process continued\n", stderr);   
      status = cached_errno;
   }
   return status; 
}
static int exec_simple( command_t* cmd ) {
   int state = 0;
   pid_t pid = fork();
   switch( pid ) {
      case -1:
         state = errno;      
         break;
      case 0:
         
        (void)sef_def_signal();
        if(cmd->env) setenv(cmd->env->key, cmd->env->value, true );
        execvp( cmd->name, cmd->args );
        _exit(EXIT_FAILURE);

        break;
      default: 
         state = parent_wait( pid );   
   } 
   return state;
}

static int exec_redirected( command_t* cmd ) {
   int state = 0;

   return state;
}

int execute( command_t* head ) {
   command_t* tmp = head;
   pid_t pid;
   int status = 0;

   while( tmp ) {
      switch( tmp->type ) {
         case ENVIRON:
            (void)set_shell_var( tmp->env );
            break;
         case SIMPLE:
            if((status = exec_builtin( tmp->name, tmp->args )) == ENOEXT ) 
               status = exec_simple( tmp );
            break;
         case REDIRECTED:
            status = exec_redirected( tmp );
            break;         
         case GROUP:
            break;
        default:
         fprintf( stderr, "Unknown type: %s", tmp->name );
         status = -1;
      }
      tmp = tmp->next;
   }
   return status;
}
