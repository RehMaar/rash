#include "lexer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int is_metachar( char c ) {
   int state = true;
   switch( c ) {
      case ';': break;
      case '|': break;
      case '>': break;
      case '<': break;
      case '\'': break;
      case '\"': break;
      case '&': break;
      case ')': break;
      case '(': break;
      default: state = false;
   }
   return state;
}

int is_shell_var( char c ) {
   int state = true;
   switch( c ) {
      case '?': break;
//      case '!': break;
      case '$': break;
      case '=': break;
      default: state = false;
   }
   return state;
}

int is_blank( char c ) {
   int state = true;
   switch( c ) {
      case ' ': break;
      case '\n': break;
      case '\t': break;
      default: state = false;
   }
   return state;
}

int is_empty( const char* line ) {
   char* tmp = (char*)line;
   int i = 0;
   while(tmp[i] != '\0'){
      if( tmp[i] > ' ' ) return false;
      i++;
   }
   return true;
}

static void clear_buffers( command_t* buf, environ_t** env, redir_map_t** redir ){
   buf->env  = NULL;                  
   buf->name = NULL;                  
   buf->args = NULL;                  
   buf->next = NULL;                  
   buf->redir_map = NULL;
   buf->type = 0;
   *env = NULL;
   *redir = NULL; 
}
   
static void write_buffer(command_t* buf,char** tok,
                        int s,int e,cmd_type_t t ){
    int length = e-s+2; 
    buf->name = strdup( tok[s] ); 
    buf->args = ALLOCATE(char*, length);
    buf->type = t;
    for( int iter = 0; iter < length-1; iter++ ) {
       buf->args[iter] = strdup(tok[s+iter]);
    }
    buf->args[length-1] = NULL;
}

int parse_cmd( char** tokens, command_t** head ) {

   command_t buffer;
   environ_t* env; 
   redir_map_t* redir;

   char* line = NULL, *prev = NULL;
   int i = 0, cmd_s = 0, cmd_e = 0, length, r_offset = 0;
    
   clear_buffers( &buffer, &env, &redir );
   while((line = tokens[i])) {
      if( line[0] == DOLLAR && strlen(line) != 0) {
         char* env;
         char* tmp = strtok( line, "$");
         if( tmp ) {
            if((env  = getenv(tmp))|| ( env = get_shell_var( tmp ))) {
               free(tokens[i]);
               tokens[i] = strdup(env);
            }
         }
         else if( line[1] == DOLLAR ) {
            char buf[2];
            sprintf(buf, "%c", DOLLAR );
            if(( env = get_shell_var( buf ))) {
               free(tokens[i]);
               tokens[i] = strdup(env);
            }
         }
      }
      else if( line[0] == '\"') { /* TODO: sub_env */
         char* buf;
         char* tmp = strtok(line, "\"\"");
         if( tmp ) {
             buf = strdup( tmp );
         }
         else {
            buf = strdup("");
         }
         free(tokens[i]);
         tokens[i] = buf; 
      } 
      else if( line[0] == '\'') {
         char* buf;
         char* tmp = strtok(line, "''");
         if( tmp ) {
            buf = strdup( tmp );
         }
         else  {
            buf = strdup( "" );
         }
         free(tokens[i]);
         tokens[i] = buf; 
      }
      i++;
   }
   i=0;
   prev = tokens[i];
   while((line = tokens[i])) {
      if(strcmp( line, ";") == 0 ) {
         if( cmd_s <= cmd_e ) {
            write_buffer(&buffer,tokens,cmd_s,cmd_e, SIMPLE);
         }
            cmd_list_add_back( head, &buffer);            
            clear_buffers( &buffer, &env, &redir );
            cmd_s = i+1; 
      }
      else if(strcmp( line, ">") == 0) {
         redir_map_t tmp_redir;
         if( strcmp( prev, "2") == 0 ) {
            tmp_redir.source.fd = STDERR;
            if( redir == NULL )cmd_e = i-2;
         }
         else {
            tmp_redir.source.fd = STDOUT;
            if( redir == NULL )cmd_e = i-1;
         }
         tmp_redir.type_source = FDW;

#if DEBUG == 1
         printf( "Source: %d Type: %d\n", 
                  tmp_redir.source.fd, tmp_redir.type_source ); 
#endif
         if((line = tokens[i+1])) {
            i++;
            if(strcmp(line, "&") == 0) {
               if((line = tokens[i+1])) {
                  i++;
                  if( strcmp(line, "1") == 0 ){
                     tmp_redir.target.fd = STDIN;
                     tmp_redir.type_target = FDW;
#if DEBUG == 1
                     printf( "Target: %d Type: %d\n", 
                        tmp_redir.target.fd, tmp_redir.type_target ); 
#endif
                     }
                  else if( strcmp(line, "2") == 0) {
                     tmp_redir.target.fd = STDERR;
                     tmp_redir.type_target = FDW;
#if DEBUG == 1
                     printf( "Target: %d Type: %d\n", 
                        tmp_redir.target.fd, tmp_redir.type_target ); 
#endif            
                  }
                  else
                     return EPARSE;

               }
               else
                  return EPARSE;
            }        
            else {
               tmp_redir.target.name = strdup(line);
               tmp_redir.type_target = FILENAME;
           }
            redir_map_add_back( &redir, &tmp_redir);         
            buffer.redir_map = redir;
         }
         else 
            return EPARSE;
      }
      else if(strcmp( line, "<") == 0) {
         redir_map_t tmp_redir;
         tmp_redir.source.fd = STDIN;
         if( redir == NULL )cmd_e = i-1;
         tmp_redir.type_source = FDR;

         if((line = tokens[i+1])) {
            i++;
            tmp_redir.target.name = strdup(line);
            tmp_redir.type_target = FILENAME;
        }
         else {
            return EPARSE;
         }
         redir_map_add_back( &redir, &tmp_redir);         
         buffer.redir_map = redir;
      }
      else if((strcmp(line, "=") == 0) && i == (cmd_s+1)) {
         if( strlen(prev) == 1 && 
               (is_metachar(prev[0]) || is_shell_var(prev[0]))) {
            return EPARSE;
         }
         env = add_back_environ_map( env, prev, tokens[i+1]); 
         buffer.env = env;
         cmd_s = i+2;
      }
      else {
         cmd_e = i;
      }
      prev = line;
      i++;
   }
   if( cmd_s <= cmd_e ) {
      write_buffer(&buffer,tokens,cmd_s,cmd_e,SIMPLE);
      cmd_list_add_back( head, &buffer);                  
   }
   else if( env ) {
      buffer.type = ENVIRON;
      cmd_list_add_back( head, &buffer);                  
   }
   return 0;
}
