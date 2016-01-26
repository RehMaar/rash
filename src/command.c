#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utillib.h"
#include "command.h"
#include "environ.h"
#include "builtin.h"

static command_t *cmd_list_create( command_t* list ){
	command_t *node;
   if( list == NULL ) return NULL;
	node = ALLOCATE(command_t, 1);
   node->env = list->env;
   node->name = list->name;
   node->args = list->args;
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

void print_commands( const command_t* head ) {
   if( head ) {                                                         
      command_t* head_tmp = (command_t*)head;                                     
      while( head_tmp ) {                                             
         puts( "New command" );
         if(head_tmp->name) printf("Name: %s\n", head_tmp->name);   
         if(head_tmp->args) {                                         
            int head_iter = 0;                                        
            while(head_tmp->args[head_iter]) {                      
               printf("Args: %s\n", head_tmp->args[head_iter++]);   
            }                                                           
         }                                                              
         if(head_tmp->env) {                                          
           environ_t* head_env = head_tmp->env;                     
           while( head_env ) {                                        
               printf("Environ: %s -- %s\n", head_env->key, head_env->value);  
               head_env = head_env->next;                           
           }                                                            
         }                                                              
         head_tmp = head_tmp->next;                                 
      }                                                                 
   }
}

error_t execute( command_t* head, int *stat ) {

   error_t state = 0; 
   command_t* tmp = head;
   *stat = 0;

   while( tmp ) {
      if( tmp->name == NULL ) {
         (void)set_shell_var( tmp->env );
      }
      else if( exec_builtin( tmp->name, tmp->args ) != ENOEXT ) {
         *stat = state;
      }
      else {
         pid_t pid;
         int status;

         pid = fork();
         if( pid == 0 ) {
            if(tmp->env) 
               setenv(tmp->env->key, tmp->env->value, true );
            execvp( tmp->name, tmp->args );
            _exit(EXIT_FAILURE);
         }
         else if( pid < 0 ) {
            state =  errno; 
         }
         else {
            waitpid( pid, &status, 0 );
            if(status != 0) fprintf( stderr, "Error: %d\n", status);
         }        
      }
      tmp = tmp->next;
   }
   return state;
}
/*
error_t execute( command_t* head, int *stat ) {
   error_t state = 0; 
   command_t* tmp = head;
   pid_t pid;
   int status;
   *stat = 0;

   while( tmp ) {
      if()   
   }
}
*/
