#include "parser.h"

/**
 * Comprueba que los argumentos pasados son validos
 * tanto el numero, como que se trate de un puerto valido 
 */
t_error_code parser_arg_check(int argc, char **argv)
{
	t_error_code err; 

	err = parser_check_arg_count(argc); 
	if(err == SUCCESS) 
		err = parser_check_port(argv[1]); 
	return (err); 
}


/**
 * Comprueba que el numero de arguemntos pasado sea el correcto
 */
t_error_code parser_check_arg_count(int argc)
{
	if (argc < 2)
		return (EXT_NO_ARG); 
	if (argc > 2)
		return (EXT_TO_MANY_ARG); 
	return (SUCCESS); 
}



/**
 * Comprueba que el puerto introducido por el usuairo seea correcto
 * Concretamente que sea un puerto entre 1024-65535, evitando que se
 * pase como argumento puertos reservados o cualquier cadena de texto 
 * que no sea un numero
 */

t_error_code parser_check_port(char *argv)
{
	int port; 

	if(sscanf(argv, "%d", &port) == 0)
		return (EXT_BAD_PORT); 


	if(port < 1024 || port > 65535)
		return(EXT_BAD_PORT); 

	return(SUCCESS); 
}