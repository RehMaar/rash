#include <regex.h>
#include <stdio.h>
#include "rc_parser.h"

#if DEBUG
#include <time.h>
#endif

static FILE* get_next( const char* pathname ) {
   if(access(pathname, R_OK) != 0 ) return NULL;
   return fopen(pathname, "r");
}

error_t rc_parser( char** pathnames, env_t** env ) {
   FILE* fd = NULL; 
   char* line = NULL;
   size_t len = 0;
   ssize_t read = 0;
   error_t state = OK;
   int i = 0;
#if DEBUG
   clock_t start = clock();
#endif

   char* regstr = "^[^[:space:]]+=((\".*\")|(\'.*\')|([^[:space:]]*))$";
   char* regvalid ="^(#|[[:space:]]*)"; 
   regex_t regex_str, regex_valid;

   if(regcomp(&regex_str, regstr, REG_EXTENDED) != 0)
      return EREGCOMP;

   if(regcomp(&regex_valid, regvalid, REG_EXTENDED) != 0) {
      regfree( &regex_str);
      return EREGCOMP;
   }
  
   while(pathnames[i]) {
      if((fd = get_next(pathnames[i++])) == NULL ) continue; 

      while((read = getline(&line,&len,fd)) != -1 ) {
#if DEBUG
         printf("%s\n",line);
#endif
         size_t last = strlen(line)-1UL;
         if(line[last]=='\n') line[last] = '\0';
         if((state = regexec(&regex_str, line,0,NULL,0)) == 0 ) {
            char* save;
            char* tmp_str = strtok_r( line, "=", &save);
            printf("%s %s", tmp_str, save );
            *env = add_back_env_map( *env, tmp_str, save );        
         } 
         else if( state == REG_NOMATCH ) {
            if((state = regexec(&regex_valid,line,0,NULL,0)) == 0 ) {
            continue; 
            }
            else if( state == REG_NOMATCH ) {
               state = ENOVALIDRC;
               break;
            }
            else {
               state = ENOMATCH;
               break;
            }
         }
         else {
            state = ENOMATCH; 
            break;
         } 
      }
   }
   
   regfree(&regex_str);
   regfree(&regex_valid);
 #if DEBUG
   fprintf(stderr, "Time: %f\n", (double)(clock()-start)/CLOCKS_PER_SEC);
 #endif
   return state;
}
