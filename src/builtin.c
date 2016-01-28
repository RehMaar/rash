#include "utillib.h"
#include "builtin.h"
#include "environ.h"
#include "lexer.h"
#include "config.h"

//#define  _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int export( char** );
static int exec( char** );
static int cd( char** );
static int pwd( char** );
static int rashenv( char** );
static int rashhelp( char** );
static int umask_builtin( char** );

#define USAGE( INDEX ) \
   fprintf( stderr, "Usage: %s\n", builtins_list[INDEX].usage );  

#define BUILTINS_COUNT 7

enum {
   EXPORT = 0,
   CD,
   PWD,
   RASHENV,
   UMASK,
   RASHHELP,
   EXEC
};

typedef int(*builtin_func_t ) ( char** );
struct _builins_t {
   char* name;
   char* description;
   char* usage;
   builtin_func_t func;
} builtins_list[ BUILTINS_COUNT] = {
   { 
      "export",    
      "export environment variables", 
      "export NAME=VAR...",
      export   
   },
   { 
      "cd",        
      "change current working directory",
      "cd [ - | path ]",   
      cd       
   },
   { 
      "pwd",       
      "print current working directory",
      "pwd",
      pwd      
   },
   { 
      "rashenv",   
      "print rash's environment variables",
      "rashenv [NAME]",
      rashenv  
   },
   { 
      "umask",     
      "",
      "",
      umask_builtin
   },
   { 
      "rashhelp",  
      "print this message",
      "rashhelp [name...]",
      rashhelp    
   },
   { 
      "exec",
      "execute script file ( inline scripts )",  
      "exec filename",
      exec
   }
};

int is_builtin( char* name ) {
   for(int i = 0; i < BUILTINS_COUNT; i++) {
      if( strcmp( builtins_list[i].name, name ) == 0){
         return i;
      }
   }
   return -1; 

}
int execute_builtin( int index, char** args ) {
   return builtins_list[index].func( args );
}

/*
int execute_builtin( char* name, char** args ) {
   int state;
   for(int i = 0; i < BUILTINS_COUNT; i++) {
      if( strcmp( builtins_list[i].name, name ) == 0){
         state = builtins_list[i].func( args );
         return state;
      }
   }
   return ENOEXT; 
}
*/
static int rashhelp( char** args ) {
   for( int i = 0; i < BUILTINS_COUNT; i++)
         printf( "> %s -- %s\nUsage: %s\n", 
                  builtins_list[i].name,
                  builtins_list[i].description,
                  builtins_list[i].usage
               );
   return SUCCESS;
}

static int umask_builtin( char** args ) {
   mode_t mask = 0; size_t i = 0UL;
   if( args[1]) {
      size_t len = strlen(args[1])-1UL;
      while( i <= 2UL && i <= len ) {
         switch( i ) {
            case 0UL:
               switch( args[1][len-i] ) {
                  case '0': break;
                  case '1': mask |= S_IXOTH; break;
                  case '2': mask |= S_IWOTH; break;
                  case '3': mask |= S_IXOTH|S_IWOTH; break;
                  case '4': mask |= S_IROTH; break;
                  case '5': mask |= S_IROTH|S_IXOTH; break;
                  case '6': mask |= S_IROTH|S_IWOTH; break;
                  case '7': mask |= S_IROTH|S_IWOTH|S_IXOTH; break;
                  default:
                     fprintf(stderr, "Bad mask: %c\n", args[1][len]);
                     return -1;
               }
               break;
            case 1UL:
              switch( args[1][len-i] ) {
                  case '1': mask |= S_IXGRP; break;
                  case '2': mask |= S_IWGRP; break;
                  case '3': mask |= S_IXGRP|S_IWGRP; break;
                  case '4': mask |= S_IRGRP; break;
                  case '5': mask |= S_IRGRP|S_IXGRP; break;
                  case '6': mask |= S_IRGRP|S_IWGRP; break;
                  case '7': mask |= S_IRGRP|S_IWGRP|S_IXGRP; break;
                  default:
                     fprintf(stderr, "Bad mask: %c\n", args[1][len]);
                     return -1;
               } 
               break;
            case 2UL:
               switch( args[1][len-i] ) {
                  case '1': mask |= S_IXUSR; break;
                  case '2': mask |= S_IWUSR; break;
                  case '3': mask |= S_IXUSR|S_IWUSR; break;
                  case '4': mask |= S_IRUSR; break;
                  case '5': mask |= S_IRUSR|S_IXUSR; break;
                  case '6': mask |= S_IRUSR|S_IWUSR; break;
                  case '7': mask |= S_IRUSR|S_IWUSR|S_IXUSR; break;
                  default:
                     fprintf(stderr, "Bad mask: %c\n", args[1][len]);
                     return -1;
               }
               break;
            default:
               fputs( "umask: Unknown error", stderr);
         }
         i++; 
      }
      printf( "%o\n", mask );
      (void)umask( mask );  
   }
   else {
#ifndef _GNU_SOURCE
      mask = umask(0);
      (void)umask(mask);
#else 
      mask = getumask();
#endif
      printf("%4o\n", mask);  
   }
   return 0;
}


/* rashenv -- args[0], args[1] -- NULL or key */
static int rashenv( char** args ) {
   if( args[1] ) {
      char* value = get_shell_var( (const char*)args[1]);
      if( value )
         printf("%s=%s\n", args[1], value );
      else {
         fprintf( stderr, "No variable: %s.", args[1] );
         return -1;
      }
   } 
   else {
      print_shell_var();
   }
   return 0;
}

static int pwd( char** args ) {
   char* cwd = getenv("PWD");
   if( cwd ) {
      puts( cwd );
      return SUCCESS;
   }
   else {

      USAGE( PWD )
      perror("pwd");
      return ERROR;
   }
}

static int cd( char** args ) {
   int state;
   char* dir = NULL;
   if( args[1]) {
      if(strcmp(args[1], "-") == 0) {
         if((dir = getenv("OLDPWD")) == NULL) 
            dir = getenv("HOME");
      }
      else 
         dir = args[1];
   }
   state = chdir(dir);
   if( state == -1 ) {
      USAGE( CD )
      perror("cd");
   }
   else {
      set_cwd();
   }
   return 0;   
}

static int exec( char** args ) {
   return  read_script((const char*)args[1]); 
}

static int export( char** args ) {
   int state = -1;
   int i = 1;
   while( args[i] ) {
      if( strlen(args[i-1]) == 1 && args[i-1][0] == '=' && i != 2 ) {
         state = setenv(args[i-2], args[i], 1 );     
      }
      i++;
   }
   return state;
} 
