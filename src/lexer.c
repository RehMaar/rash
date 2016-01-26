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
//      case '&': break;
//      case ')': break;
//      case '(': break;
      default: state = false;
   }
   return state;
}

int is_shell_var( char c ) {
   int state = true;
   switch( c ) {
      case '?': break;
      case '!': break;
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

#define CLEAR_BUFFERS( buf, envir )  \
   buf.env  = NULL;                  \
   buf.name = NULL;                  \
   buf.args = NULL;                  \
   buf.next = NULL;                  \
   envir = NULL;

#define WRITE_BUFFER( buf, tok, start, end )             \
    length = end-start+2;                                \
    buf.name = strdup( tok[start] );                     \
    buf.args = ALLOCATE(char*, length);                  \
    for( int iter = 0; iter < length-1; iter++ ) {       \
       buf.args[iter] = strdup(tok[start+iter]);         \
    }                                                    \
    buf.args[length-1] = NULL;

error_t parse_cmd( char** tokens, command_t** head ) {

   command_t buffer;
   environ_t* env = NULL; 
   char* line = NULL, *prev = NULL;
   int i = 0, cmd_s = 0, cmd_e = 0, length;
    
   CLEAR_BUFFERS( buffer, env )
   while((line = tokens[i])) {
      if( line[0] == '$' && strlen(line)!=0) {
         char* tmp = strtok( line, "$");
         char* env;
         if((env  = getenv(tmp))|| ( env = get_shell_var( tmp ))) {
            free(tokens[i]);
            tokens[i] = strdup(env);
         }
      }
      else if( line[0] == '\"') { /* TODO: sub_env */
         char* tmp = strtok(line, "\"\"");
         char* buf = strdup( tmp );
         free(tokens[i]);
         tokens[i] = buf; 
      } 
      else if( line[0] == '\'') {
         char* tmp = strtok(line, "''");
         char* buf = strdup( tmp );
         free(tokens[i]);
         tokens[i] = buf; 
      }
      i++;
   }
   i=0;
   prev = tokens[i];
   while(( line = tokens[i] )) {
      if( strlen(line) == 1) {
         if( line[0] == ';' ) {   
            if( cmd_s <= cmd_e ) {
               WRITE_BUFFER(buffer,tokens,cmd_s,cmd_e)
            }
            cmd_list_add_back( head, &buffer);            
            CLEAR_BUFFERS( buffer, env )
            cmd_s = i+1; 
         }
         else if( line[0] == '=' && i == (cmd_s+1)) {
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
       }
       else {
         cmd_e = i;
       }
       prev = line;
       i++;
   }
   if( cmd_s <= cmd_e ) {
      WRITE_BUFFER(buffer,tokens,cmd_s,cmd_e)
      cmd_list_add_back( head, &buffer);                  
   }
   else if( env ) {
      cmd_list_add_back( head, &buffer);                  
   }
   return SUCCESS;
}
