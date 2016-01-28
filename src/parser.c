#include "parser.h"
#include "lexer.h"
#include <ctype.h>
static int tokens_count( const char* test_str ) {

   int count = 0;
   int i; char c, last_char;
   for( i=0, last_char = test_str[i]; test_str[i] != ZERO; i++) {
      c = test_str[i];
      if( c == QWMARK || c == QQMARK ) {
         char mark = c; i++;
         while(( c = test_str[i]) != mark ) {
            if( c == ZERO ) return -1; 
            i++;
         }
         count++; 
      } 
      else if( c == DOLLAR ) {
         count++;i++;
         if(( c = test_str[i++])) {
            while(is_blank(c) || is_metachar(c) || ispunct(c)) {
               if( c == ZERO ) return count;
               c = test_str[i++];
            }
         }
      }
      else if( is_metachar(c) || is_shell_var(c) ) {
         count++;
      }
      else if( !is_blank(c) ) {
         if(is_metachar(last_char) || is_shell_var(last_char) || 
            is_blank(last_char) || i == 0){
            count++;
         }     
      }
      last_char = c;
   }
   return count; 
} 

char** splittok( const char* line ) {
   char** tokens;
   int i = 0, j = 0;
   char c, last_char;
   char buffer[256];
   char* tmp = (char*)line;
   int count = tokens_count( line );
  
   if( count <= 0 )  return NULL;
   tokens = calloc(sizeof(char*),(count+1));
   c = last_char = tmp[0];
   for(; j < count; j++ ) {
      while((c = tmp[i++])) {
         if ( c == QWMARK || c == QQMARK ) {
            char mark = c;
            int k = 0;
            do {
               if( c == ZERO ) break;
               buffer[k++] = c;
            }
            while((c = tmp[i++]) != mark);
            if(c) {
               buffer[k++] = mark;
               buffer[k] = ZERO;
            }
            break;
         }
         else if( c == DOLLAR ) {
            int k = 0;
            do {
               buffer[k++] = c;
            }
            while((c = tmp[i++]) || is_metachar(c) || is_blank(c));
            buffer[k] = ZERO;
            i--;
            break;
 
         }
         else if( is_metachar( c ) || is_shell_var( c )) {
            buffer[0] = c; buffer[1] = ZERO;
            last_char = c;
            break; 
         }
         else if( is_blank(c) == false ){
            int k=0;
            while( !(is_blank(c) == true || is_metachar(c) || is_shell_var(c))){
               buffer[k++] = c; 
               c = tmp[i++];
               if( c == ZERO ) break;
            }
            i--;
            last_char = c;
            buffer[k] = ZERO;
            break;
         }
      }
      tokens[j] = strdup(buffer);
   }
   tokens[count] = NULL;
   
   return tokens;
}

void destroy_tokens( char** tokens ) {
   if( tokens ) {
      for( int i = 0; tokens[i]; i++) free(tokens[i]); 
      free( tokens );
   }
}
