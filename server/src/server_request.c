/* ********************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para tratar las  */
/*	peticiones recibidas por el servidor    							  */
/* ********************************************************************** */

#include "server_request.h"

t_error_code server_request_register(int sc_copy, t_request *request, t_response *response)
{
	if(ERROR == readLine(sc_copy, request->user.full_name, MAX_SIZE))
		return (RECEIVE_ERROR); 

	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(ERROR == readLine(sc_copy, request->user.date, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_register(request);
	request_status_msg(REGISTER); 
		
	return (SUCCESS);
} 

t_error_code server_request_unregister(int sc_copy, t_request *request, t_response *response)
{
	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_unregister(request->user.alias);
	request_status_msg(UNREGISTER); 

	return (SUCCESS);
} 

t_error_code server_request_connect(int sc_copy, t_request *request, t_response *response)
{

	char *messages; 

	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	if(ERROR == readLine(sc_copy, request->user.port, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	response->status = db_connect(request, response);
	request_status_msg(CONNECT); 

	// pillar los mensajes pendientes, si existen bucle enviando los mensajes

	return (SUCCESS);			
}

t_error_code server_request_disconnect(int sc_copy, t_request *request, t_response *response)
{
	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_disconnect(request, response);
	request_status_msg(DISCONNECT); 

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