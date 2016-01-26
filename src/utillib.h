#ifndef COMLIB_H
#define COMLIB_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/* Useful things. I hope. */
#define TEST( condition ) if( condition ) return errno;

/* Memmory managing. */
#define ALLOCATE( type, count) (type*)calloc(sizeof(type), count)

#define FREE_MATRIX( ptr ) if( ptr ) {                            \
   int ptr##_i = 0; while( ptr[ptr##_i]) free(ptr[ptr##_i++]);    \
   free(ptr);                                                     \
}

#define SHOW( array )                  \
   if( array ) {                       \
      int array##_iter = 0;            \
      while(array[ array##_iter]) {    \
         puts( array[array##_iter++]); \
      }                                \
   }
 

/* State macro */

#define SUCCESS       0 
#define ERROR        -1
#define ENOVALIDRC   -2
#define ENOMATCH     -3
#define EPARSE       -4
#define ENOEXT       -5


typedef int error_t;
/*
typedef struct {
   char* util;
   int   code; 
} error_t;
*/
/*
typedef enum {
   SUCCESS,
   ENOVALIDRC,
   ENOMATCH,
   EPARSE
} error_t;
*/

#define true 34
#define false 0


#endif /* COMLIB_H */
