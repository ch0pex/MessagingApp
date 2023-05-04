#include "parser.h"

t_error_code parser_arg_check(int argc, char **argv)
{
	t_error_code err; 

	err = parser_check_arg_count(argc); 
	if(err == SUCCESS) 
		err = parser_check_port(argv[1]); 
	return (err); 
}

t_error_code parser_check_arg_count(int argc)
{
	if (argc < 2)
		return (EXT_NO_ARG); 
	if (argc > 2)
		return (EXT_TO_MANY_ARG); 
	return (SUCCESS); 
}

t_error_code parser_check_port(char *argv)
{
	int port = atoi(argv); 
	if(port < 1024 || port > 49151)
		return(EXT_BAD_PORT); 

	return(SUCCESS); 
}