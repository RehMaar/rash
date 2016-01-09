#include "parser.h"
#include <string.h>

size_t count_tokens( char* str, const char delim ) {
   size_t count = 0; 
   char last_char = ' ';
   for( size_t i = 0; str[i]; i++ ) {
      if( str[i] == delim && last_char != delim)
         count++;
      last_char = str[i]; 
   }
   if(count != 0 || last_char != delim) {
      count++;
      count += last_char == delim ? -1: 0;
   }
   return count; 
}
char** split( char* line, const char delim, size_t* count) {
   size_t c = count_tokens( line, delim );
   char* tmp, *save;
   char** tokens; 
   char delims[2]; delims[0] = delim; delims[1] = 0;
   size_t i = 0;
   *count = c;
   if(c == 0 ) return NULL;
   tokens = ALLOCATE( char*, c);
   tmp = strtok_r( line, delims, &save);
   while(tmp != NULL ) {
      tokens[i++] = strdup(tmp);
      tmp = strtok_r(NULL, delims, &save);
   }
   return tokens;
}
