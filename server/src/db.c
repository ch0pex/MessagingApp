/* ************************************************************************* */
/* Este fichero contiene las funciones necesarias para el manejo de ficheros */
/* donde se guardara todos los datos de los distintos usuarios               */ 
/* ************************************************************************* */

#include "db.h"

t_response_status db_register(t_request *request)
{
	return (OK); 
}

t_response_status db_unregister(char *username)
{
	return (OK); 
}

t_response_status db_connect(t_request *request)
{
	return (OK); 
} 

t_response_status db_disconnect(char *username)
{
	return (OK); 
}

t_response_status db_send_message(t_request *request)
{
	return (OK); 
}

t_response_status db_connected_users()
{
	return (OK); 
}

bool db_user_exist(char *user)
{
	return (OK); 
}