#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

#include "error_code.h"


t_error_code parser_arg_check(int argc, char **argv);
t_error_code parser_check_arg_count(int argc);
t_error_code parser_check_port(char *argv); 


#endif


