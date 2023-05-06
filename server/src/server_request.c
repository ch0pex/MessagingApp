/* ********************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para tratar las  */
/*	peticiones recibidas por el servidor    							  */
/* ********************************************************************** */

#include "server_request.h"

t_error_code server_request_register(int sc_copy, t_request *request, t_response *response)
{
	/* Read user info
	if(RECEIVE_ERROR == recvMessage(sc_copy, (char *) &request.data.key, sizeof(int32_t)))
		return(RECEIVE_ERROR)
	*/

	

	response->status = db_register(/*User info*/);
	return (SUCCESS);
} 

t_error_code server_request_unregister(int sc_copy, t_request *request, t_response *response)
{
	return (SUCCESS);
} 

t_error_code server_request_connect(int sc_copy, t_request *request, t_response *response)
{
	return (SUCCESS);
}

t_error_code server_request_disconnect(int sc_copy, t_request *request, t_response *response)
{
	return (SUCCESS);
}

t_error_code server_request_send_message(int sc_copy, t_request *request, t_response *response)
{
	return (SUCCESS);
} 

t_error_code server_request_connected_users(int sc_copy, t_request *request, t_response *response)
{
	return (SUCCESS);
}