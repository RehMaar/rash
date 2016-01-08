#ifndef COMLIB_H
#define COMLIB_H

/* Headers */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/* Useful things. I hope. */
#define TEST( condition ) if( condition ) return errno;

/* Memmory managing. */
#define ALLOCATE( type, count) (type*)calloc(sizeof(type), count)

/* State macro */
#define OK 0 
#define ENONVALIDRC -1
#define EREGMATCH -2

/* Types. */
typedef int error_t;

/* Debug*/

#define DEBUG 1

#endif /* COMLIB_H */
