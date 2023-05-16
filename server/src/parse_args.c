/* **************************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para el manejo de      */
/*  los argumentos pasados al ejecutable                                        */                                              
/* **************************************************************************** */

#include "parser.h"

/**********************************************************
 * Function: Comprueba que los argumentos pasados al 
 * ejecutable sean validos
 *********************************************************/
t_error_code parser_arg_check(int argc, char **argv)
{
	t_error_code err; 

	err = parser_check_arg_count(argc); 
	if(err == SUCCESS) 
		err = parser_check_port(argv[2]); 
	return (err); 
}

/**********************************************************
 * Function: Comprueba que el numero de arguemntos pasado 
 * sea el correcto
 *********************************************************/
t_error_code parser_check_arg_count(int argc)
{
	if (argc < 3)
		return (EXT_NO_ARG); 
	if (argc > 3) 
		return (EXT_TO_MANY_ARG); 
	return (SUCCESS); 
}

/**********************************************************
 * Function: Comprueba que el puerto introducido sea un 
 * numero y este comprendido entre 1024 y 65535
 *********************************************************/
t_error_code parser_check_port(char *argv)
{
	int port; 
	printf("%s\n", argv);
	if(sscanf(argv, "%d", &port) == 0)
		return (EXT_BAD_PORT); 

	if(port < 1024 || port > 65535) 
		return(EXT_BAD_PORT); 

	return(SUCCESS); 
}