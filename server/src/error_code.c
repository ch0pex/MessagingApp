#include "error_code.h"

/**
 * @brief Escribe el error por pantalla
 *
 * @param err Codigo de error
 * @return El propio error
 *
 * Esta función suma los dos números enteros proporcionados como entrada y devuelve el resultado.
 */

t_error_code error_code_print_msg(t_error_code err)
{
	switch(err)
	{
	case ERROR: 		
		printf(INTERNAL_ERROR_MSG); break; 
	case SUCCESS: 	 	
		break; 
	case SOCKET_ERROR: 	
		printf(SOCKET_ERROR_MSG); break; 
	case BIND_ERROR: 
		printf(BIND_ERROR_MSG);   break; 
	case LISTEN_ERROR: 
		printf(LISTEN_ERROR_MSG); break; 
	case ACCEPT_ERROR: 
		printf(ACCEPT_ERROR_MSG); break; 
	case EXT_NO_ARG: 
		printf(EXT_NO_ARG_MSG);   break; 
	case EXT_TO_MANY_ARG: 
		printf(EXT_TO_MANY_ARG_MSG);  break; 
	case EXT_BAD_PORT: 
		printf(EXT_BAD_PORT_MSG);  break;
	}
	return (err); 
}