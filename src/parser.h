#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <malloc.h>

char** splittok( const char* line );
int is_empty( const char* line );
void destroy_tokens( char** tokens );

#endif /* PARSER_H */
