/* ************************************************************************* */
/* Este fichero contiene las funciones necesarias para el manejo de errores  */
/* ************************************************************************* */

#include "error_code.h"

/**********************************************************
 * Function: Esta funcion escribe por pantalla el error 
 * recibido
 *********************************************************/
t_error_code error_code_print_msg(t_error_code err)
{
	switch(err)
	{
	case ERROR: 			printf(INTERNAL_ERROR_MSG); break; 	 // ERROR  -1 
	case SUCCESS: break; 										 // EXITO 0
	case SOCKET_ERROR: 		printf(SOCKET_ERROR_MSG); break; 	 // ERROR SOCKET 1
	case BIND_ERROR: 		printf(BIND_ERROR_MSG);   break; 	 // ERROR BIND 2
	case LISTEN_ERROR: 		printf(LISTEN_ERROR_MSG); break; 	 // ERROR LISTEN 3
	case ACCEPT_ERROR: 		printf(ACCEPT_ERROR_MSG); break; 	 // ERROR ACCEPT 4
	case SEND_ERROR: 		printf(SEND_ERROR_MSG); break;   	 // ERROR SEND 5
	case RECEIVE_ERROR: 	printf(RECEIVE_ERROR_MSG); break;	 // ERROR RECEIVE 6
	case HOSTNAME_ERROR: 	printf(HOST_ERROR_MSG); break;   	 // ERROR HOST 7
	case CONNECT_ERROR: 	printf(CONNECT_ERROR_MSG); break; 	 // ERROR CONNECT 8
	case EXT_TO_MANY_ARG: 	printf(EXT_TO_MANY_ARG_MSG);  break; // ERROR EXTERONO MUCHOS ARGUMENTOS 9
	case EXT_NO_ARG: 		printf(EXT_NO_ARG_MSG);   break;     // ERROR EXTERNO FALTA ARGUMENTO 10
	case EXT_BAD_PORT: 		printf(EXT_BAD_PORT_MSG); break;     // ERROR EXTERNO PUERTO ERRONEO 11
	}
	return (err); 
}
