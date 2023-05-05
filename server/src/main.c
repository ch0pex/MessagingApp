/* ***************************************************************************** */
/* Este fichero contiene la funcion main del programa donde se lanza el servidor */
/* ***************************************************************************** */

#include "main.h"

/**********************************************************
 * Function: Funcion main, lanza el servidor
 *********************************************************/

int main(int argc, char **argv)
{
	t_error_code err; //Enum con los distintos errores manejados
	t_server server; //Struct server 

	err = parser_arg_check(argc, argv); //Comprobacion de parametros

	if (err == SUCCESS)
	{
		err = server_init(&server, argv[1]); //Inicializacion del servidor
		if (err == SUCCESS)
		{
			err = server_loop(&server); // Bucle del servidor
		}
	}
	
	return (error_code_print_msg(err)); 
}


