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
       if( tok[s+iter][0] == '\"') { // TODO: sub_env
         char* buf;
         char* tmp = strtok(tok[s+iter], "\"\"");
         if( tmp ) {
             buf = strdup( tmp );
         }
         else {
            buf = strdup("");
         }
         free(tok[s+iter]);
         tok[s+iter] = buf; 
      } 
      else if( tok[s+iter][0] == '\'') {
         char* buf;
         char* tmp = strtok(tok[s+iter], "''");
         if( tmp ) {
            buf = strdup( tmp );
         }
         else  {
            buf = strdup( "" );
         }
         free(tok[s+iter]);
         tok[s+iter] = buf; 
      }
      buf->args[iter] = strdup(tok[s+iter]);
    }
    buf->args[length-1] = NULL;
}

#define CMP( line, tok ) if(( strcmp( line, tok ) == 0 ))
int parse_cmd( char** tokens, command_t** head ) {
   command_t buffer;   /* Buffer for each command. */
   environ_t* env;     /* Environment map for each command. */
   redir_map_t* redir; /* Redirection map for each command. */
   
   char* line = NULL,  /* Current line.                     */
       * prev = NULL;  /* Previous line.                    */
   int i     = 0,      /* Command iterator.                 */
       cmd_s = 0,      /* Command start.                    */
       cmd_e = 0;      /* Command end.                      */
   
   
   clear_buffers( &buffer, &env, &redir );    
   
   while(( line = tokens[i] )) {
      if( line[0] == DOLLAR && strlen(line) != 0) {
#if DEBUG == 1
         printf( "Environ: `%s'\n", line );
#endif
         char* env_buf;
         char* tmp = strtok( line, "$");
         if( tmp ) {
            if((env_buf  = getenv(tmp))|| ( env_buf = get_shell_var( tmp ))) {
               free(tokens[i]);
               tokens[i] = strdup(env_buf);
            }
         }
         else if( line[1] == DOLLAR ) {
            char buf[2];
            sprintf(buf, "%c", DOLLAR );
            if(( env_buf = get_shell_var( buf ))) {
               free(tokens[i]);
               tokens[i] = strdup(env_buf);
            }
         }
      }
     
      CMP( line, ";" ) 
      {
#if DEBUG == 1
         printf( "Terminate `;' : %s\n", line );
#endif
         if(!(buffer.name && buffer.args)) {
            if( !buffer.env ) {
               if( redir ) destroy_redir_map( redir );   
               return EPARSE;
            }
         }
         if( cmd_s <= cmd_e ) {
            write_buffer(&buffer,tokens,cmd_s,cmd_e, SIMPLE);
         }
         else if( buffer.env ) {
            buffer.type = ENVIRON;
         }
         cmd_list_add_back( head, &buffer);            
         clear_buffers( &buffer, &env, &redir );
         cmd_s = i+1; 
      }
      else CMP( line, "|" )
      {
#if DEBUG == 1
         printf( "Pipeline `|' : %s\n", line );
#endif
         if(!(prev) || !tokens[i+1]) {
            if( env ) destroy_environ_map( env );
            if( redir ) destroy_redir_map( redir );
            clear_buffers( &buffer, &env, &redir );
            return EPARSE;
         }  
         if( cmd_s <= cmd_e ) {
            write_buffer(&buffer,tokens,cmd_s,cmd_e, PIPELINE);
            cmd_list_add_back( head, &buffer);            
            clear_buffers( &buffer, &env, &redir );
            cmd_s = i+1; 
         }
         else {
            if( env ) destroy_environ_map( env );
            if( redir ) destroy_redir_map( redir );
            clear_buffers( &buffer, &env, &redir );
            puts("1");
            return EPARSE;  
         }
      }
      /* SOURCE > TARGET */
      else CMP( line, ">") {
#if DEBUG == 1
         printf( "Redirection `>' : %s\n", line );
#endif
         redir_map_t tmp_redir;
          
         if( !prev ) return EPARSE;
         CMP( prev, "2" ) {
            tmp_redir.source.fd = STDERR;
            if( !redir ) cmd_e = i - 2; /* cmd 2> i: >;i-1: 2; i-2: cmd;*/
         }
         else {
            tmp_redir.source.fd = STDOUT;    
            if( !redir ) cmd_e = i - 1; /* cmd > i: >; i-1: cmd;*/
         }
         tmp_redir.type_source = FDW; /* Write. */
   
         if(( line = tokens[i++ +1] )) {
            CMP( line, "&" ) {
               if((line = tokens[i++ +1])) {
                  CMP( line, "1" ) {
                     tmp_redir.target.fd = STDOUT; /* >&1 */
                     tmp_redir.type_target = FDW; /* Write. */
                  }
                  else CMP( line, "2" ) {
                     tmp_redir.target.fd = STDERR; /* >&2 */
                     tmp_redir.type_target = FDW;  /* Write. */
                  }
                  else {
                     if( env ) destroy_environ_map( env );
                     if( redir ) destroy_redir_map( redir );
                     clear_buffers( &buffer, &env, &redir );
                     return EPARSE;
                  }
               }   
               else {
                     if( env ) destroy_environ_map( env );
                     if( redir ) destroy_redir_map( redir );
                     clear_buffers( &buffer, &env, &redir );
                     return EPARSE;
               }
            }
            else {
               tmp_redir.target.name = strdup(line);
               tmp_redir.type_target = FILENAME; 
            }
            redir_map_add_back( &redir, &tmp_redir);         
            buffer.redir_map = redir;
         }
         else {
            if( env ) destroy_environ_map( env );
            if( redir ) destroy_redir_map( redir );
            clear_buffers( &buffer, &env, &redir );
            return EPARSE;
         }
      }
      else CMP( line, "<" ) {
#if DEBUG == 1
         printf( "Redirection `<' : %s\n", line );
#endif  
         if( !prev ) return EPARSE;
         redir_map_t tmp_redir;
         tmp_redir.source.fd = STDIN;     /* cmd SOURCE<TARGET*/
         if( redir == NULL ) cmd_e = i-1; /* i: <; i-1: cmd */
         tmp_redir.type_source = FDR;     /* Read. */
         
         if(( line = tokens[i++ +1] )) {
            tmp_redir.target.name = strdup(line);
            tmp_redir.type_target = FILENAME;
         }
         else {
            if( env ) destroy_environ_map( env );
            if( redir ) destroy_redir_map( redir );
            clear_buffers( &buffer, &env, &redir );
               puts("7");
            return EPARSE;
         }
         redir_map_add_back( &redir, &tmp_redir);         
         buffer.redir_map = redir;
      }
      else CMP( line, "=" ) {
#if DEBUG == 1
         printf( "Environ `=' : %s\n", line );
#endif
        /* NAME=VAR: NAME -- cmd_s, = -- cmd_s+1. Only first.*/
        if( i == ( cmd_s + 1 ) ) { 
            if( strlen(prev) == 1 && 
              ( is_metachar(prev[0]) || is_shell_var(prev[0]) ) ) {
               if( env ) destroy_environ_map( env );
               if( redir ) destroy_redir_map( redir );
               clear_buffers( &buffer, &env, &redir );
               puts("8");
               return EPARSE;
            } 
            env = add_back_environ_map( env, prev, tokens[i+1]); 
            buffer.env = env;
            cmd_s = i+2; /* Set new offset, */
        }    
      }
      else 
      {
#if DEBUG == 1
         printf( "Token: %s\n", line );
#endif 
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
