#ifndef COMLIB_H
#define COMLIB_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/* Useful things. I hope. */
#define TEST( condition ) if( condition ) return errno;

/* Memmory managing. */
#define ALLOCATE( type, count) (type*)calloc(sizeof(type), count)



#define SHOW( array )                  \
   if( array ) {                       \
      int array##_iter = 0;            \
      while(array[ array##_iter]) {    \
         puts( array[array##_iter++]); \
      }                                \
   }
 
/* Char's alias. */
#define ZERO         '\0'
#define QWMARK       '\''
#define QQMARK       '\"'
#define BLANK_END    ' '
#define NL           '\n'
#define DOLLAR       '$'
#define LANGLE       '<'
#define RANGLE       '>'

/* State macro */

#define SUCCESS       0 
#define ERROR        -1
#define ENOVALIDRC   -2
#define ENOMATCH     -3
#define EPARSE       -4
#define ENOEXT       -5


#define true 34
#define false 0

#define DEBUG 1

#endif /* COMLIB_H */
