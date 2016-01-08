#include <regex.h>

#include "rc_parser.h"
#if DEBUG
#include <time.h>
#endif


error_t rc_parser( const char* pathname, environ_t* env ) {

   FILE* fdi    = NULL; 
   char* line   = NULL;
   size_t len   = 0;
   ssize_t read = 0;
   int state    = 0;
#if DEBUG
   clock_t start = clock();
#endif

   regex_t regex_str, regex_valid;
   char* regstr = "^[^[:space:]]+=((\".*\")|(\'.*\')|([^[:space:]]*))$";
   char* regvalid ="(#)|([[:space:]]*))"; 


   TEST(access(pathname, R_OK) != 0 )
   fd = fopen(pathname, "r");
   TEST(fd== NULL)
   TEST(regcomp(&regex_str, regstr, REG_EXTENDED) != 0)

   if(regcomp(&regex_valid, regvalid, REG_EXTENDED) != 0){
      state = errno;
      regfree( &regex_str );
      return state;
   }
   
   while((read = getline(&line,&len,fd)) != -1 ) {
#if DEBUG
      printf("%s\n",line);
#endif
      size_t last = strlen(line)-1UL;
      if(line[last]=='\n') line[last] = '\0';
      if((ret = regexec(&regex_str, line,0,NULL,0)) == 0 ) {
         /* TODO: Understand, huh? */
      } 
      else if( ret == REG_NOMATCH ) {
         if((ret = regexec(&regex_valid,line,0,NULL,0)) == 0 ) {
           continue; 
         }
         else if( ret == REG_NOMATCH ) {
            state = ENOVALIDRC;
         }
         else {
            state = EREGMATCH;
            break;
         }
      }
      else {
         state = EREGMATCH; 
         break;
      } 
   }
   regfree(&regex_str);
   regfree(&regex_valid);
 #if DEBUG
   fprintf(stderr, "Time: %f\n", (double)(clock()-start)/CLOCKS_PER_SEC);
 #endif
   return state;
}
