#ifndef LEXER_H
#define LEXER_H

#include "utillib.h"
#include "environ.h"
#include "command.h"

int is_metachar( char c );

int is_shell_var( char c );

int is_blank( char c );


int parse_cmd( char** tokens, command_t** head );


#endif /* LAXER_H */
