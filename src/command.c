//#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "utillib.h"
#include "command.h"
#include "environ.h"
#include "builtin.h"
#include "sig_manager.h"


#define ADD_BACK_LIST( _name, _type, body )              \
static _type* _name##_create( _type* list ) {            \
   _type* node;                                          \
   if( list == NULL ) return NULL;                       \
   node = ALLOCATE(_type, 1);                            \
   body                                                  \
   return node;                                          \
}                                                        \
void _name##_add_back( _type** list, _type* tmplist ){   \
   _type* node;                                          \
   if(*list == NULL){                                    \
      *list = _name##_create(tmplist);                   \
      return;                                            \
   }                                                     \
  node = *list;                                          \
  while(node->next != NULL)                              \
    node = node->next;                                   \
  node->next = _name##_create(tmplist);                  \
}   


#define DESTROY_LIST( _name, _type, body )               \
void destroy_##_name( _type* list ) {                    \
   _type* tmp;                                           \
   while( list != NULL ) {                               \
      tmp = list->next;                                  \
      body                                               \
      list = tmp;                                        \
   }                                                     \
}

/*
 * Static functions. 
 */

static void set_pipe( int, int );
static void save_fd( int* );
static void restore_redirection( int* );
static int parent_wait( pid_t );
static int exec_builtin( command_t* );
static int exec_simple( command_t*, int* );
static int exec_pipeline( command_t** );
static int redirect( redir_map_t* );


ADD_BACK_LIST ( redir_map, redir_map_t,
   node->target = list->target;
   node->source = list->source;
   node->type_source = list->type_source;
   node->type_target = list->type_target;
   node->next = NULL;
)
DESTROY_LIST ( redir_map, redir_map_t,
   if( list->type_source == FILENAME && list->source.name ) 
      free(list->source.name);
   if( list->type_target == FILENAME && list->target.name ) 
      free(list->target.name);
   free( list );   
)

ADD_BACK_LIST( cmd_list, command_t, 
   node->env = list->env;
   node->name = list->name;
   node->args = list->args;
   node->type = list->type;
   node->redir_map = list->redir_map;
   node->next = NULL;
)
DESTROY_LIST( 
   cmd_list, command_t, 
   if( list->env )  destroy_environ_map(list->env);
      if( list->redir_map ) destroy_redir_map( list->redir_map );
      if( list->name ) free(list->name);
      if( list->args ) {
         int i = 0;
         while( list->args[i]) free(list->args[i++]);
         free( list->args );
      }
      free(list);
)

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
         if( head_tmp->redir_map ) {
            redir_map_t* tmp = head_tmp->redir_map;
            while( tmp ) {
               if( tmp->type_source == FDR || tmp->type_source == FDW )
                  printf( "> Type: %d Source FD: %d\n", 
                           tmp->type_source, tmp->source.fd );
               else 
                  printf( "> Type: %d Source file: %s\n", 
                           tmp->type_source, tmp->source.name);
               if( tmp->type_target == FDR || tmp->type_target == FDW )
                  printf( "> Type: %d Target FD: %d\n", 
                           tmp->type_target, tmp->target.fd );
               else 
                  printf( "> Type: %d Target file: %s\n", 
                           tmp->type_target, tmp->target.name);
               tmp = tmp->next;
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

static int redirect( redir_map_t* cmd ) {
   int fd_target,fd_source;
   if( cmd ) {
      redir_map_t* tmp = cmd;
       while( tmp ) {
         switch( tmp->type_source ) {
            case FDR:
               fd_source = tmp->source.fd;
               switch( tmp->type_target ) {
                  case FDR:
                  fd_target = tmp->target.fd; 
                     break;
                  case FILENAME:
                     fd_target = open( tmp->target.name, 
                                       O_RDONLY|O_CREAT,
                                       S_IRUSR| S_IWUSR|S_IRGRP|
                                       S_IWGRP|S_IROTH|S_IWOTH
                                     );
                    break;
                  default:
                     fputs("Error: unknown target type.\n", stderr);
                     return -1;
               }
               break;
            case FDW:
               fd_source = tmp->source.fd;
               switch( tmp->type_target ) {
                  case FDW:
                  fd_target = tmp->target.fd; 
                     break;
                  case FILENAME:
                     fd_target = open( tmp->target.name, 
                                       O_WRONLY|O_CREAT,
                                       S_IRUSR| S_IWUSR|S_IRGRP|
                                       S_IWGRP|S_IROTH|S_IWOTH
                                     );
                    break;
                  default:
                     fputs("Error: unknown target type.\n", stderr);
                     return -1;
               }
               break;
            default:
               fputs( "Error: unknown source type.\n", stderr);
               return -1;
         }
         if( fd_target != fd_source ) {
            dup2( fd_target, fd_source);
            close(fd_target);
         }
         tmp = tmp->next;
      }
   }
   return 0;
}

/*
 *  cmd1 | cmd2 | cmd3 | cmd4 
 *  cmd1 : pipe( fd )
 *         /dev/null -> 0 
 *         1 -> pipe fd[1]
 *  cmd2 : 
 *         pipe fd[0] -> 0
 *         1 -> pipe 
 *
 *
 *
 * fd[0] -- read
 * fd[1] -- write
 * fd [1] -> ==== -> fd[0]
 */


static int exec_pipeline( command_t** cmd ) {
   int state, fd_read, fd_write, fd[2], export_fd[2];
   fd_read = fd_write = -1;
   while( (*cmd) && (*cmd)->type == PIPELINE ) {
      pipe( fd );      
      export_fd[0] = fd_read;
      export_fd[1] = fd[1];
      
      state = exec_simple( *cmd, export_fd );
      printf( "Name: %s\nFD: %d %d\n", (*cmd)->name, export_fd[0], export_fd[1]);
      close(fd[1]);
      if( fd_read >= 0 ) close( fd_read ); 
      fd_read = fd[0];
      *cmd = (*cmd)->next;
   }
   printf( "Name: %s\n", (*cmd)->name);
   export_fd[0] = fd_read; export_fd[1] = -1;
   state = exec_simple( *cmd, export_fd );
   return state;
}
static void set_pipe( int fd_read, int fd_write ) {
   if( fd_read > 0 ) {
      dup2( fd_read, STDIN_FILENO );
      close( fd_read );
   }
   if( fd_write > 0 ) {
      dup2( fd_write, STDOUT_FILENO );
      close( fd_write );
   }
}

static int exec_simple( command_t* cmd, int* fd ) {
   int state = 0;

   if((state = exec_builtin( cmd )) != ENOEXT ) 
      return state;

   pid_t pid = fork();
   switch( pid ) {
      case -1:
         state = errno;      
         break;
      case 0:
         
         (void)set_def_signal();
         (void)redirect( cmd->redir_map );
         if(cmd->env) setenv(cmd->env->key, cmd->env->value, true );
         if( fd ) set_pipe( fd[0], fd[1]);
         execvp( cmd->name, cmd->args );
         exit(-1);

        break;
      default: 
         state = parent_wait( pid );   
   } 
   return state;
}

static void save_fd( int* fd ) {
   fd[0] = dup(0);
   fd[1] = dup(1);
}
static void restore_redirection( int* fd ) {
   dup2( fd[0], 0);  close( fd[0] );
   dup2( fd[1], 1);  close( fd[1] );
}

static int exec_builtin( command_t* cmd ) {
   int state = 0, index;
   int fd[2];
   if((index = is_builtin(cmd->name)) == -1 ) {
      return ENOEXT;
   }
   (void)save_fd( fd );
   (void)redirect( cmd->redir_map );
   (void)set_def_signal();

   state = execute_builtin( index, cmd->args );

   (void)restore_redirection( fd ); 
   (void)set_signal();
   return state;
}

int execute( command_t* head ) {
   command_t* tmp = head;
   int status = 0;

   while( tmp ) {
      switch( tmp->type ) {
         case ENVIRON:
            (void)set_shell_var( tmp->env );
            break;
         case SIMPLE:
            status = exec_simple( tmp, NULL );
            break;
         case PIPELINE:
            status = exec_pipeline( &tmp ); 
            break;
       /*
        * case GROUP:
        *    break;
        */
        default:
         fprintf( stderr, "Unknown type: %s", tmp->name );
         status = -1;
      }
      tmp = tmp->next;
   }
   return status;
}
