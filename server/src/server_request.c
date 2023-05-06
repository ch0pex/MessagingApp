/* ********************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para tratar las  */
/*	peticiones recibidas por el servidor    							  */
/* ********************************************************************** */

#include "server_request.h"

t_error_code server_request_register(int sc_copy, t_request *request, t_response *response)
{
	if(ERROR == readLine(sc_copy, request->user.user_name, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(db_user_exist(request->user.user_name))
	{
		response->status = USER_ERROR;
		request_status_msg(REGISTER); 
		return (SUCCESS); 
	}
		
	if(ERROR == readLine(sc_copy, request->user.full_name, MAX_SIZE))
		return (RECEIVE_ERROR); 

	if(ERROR == readLine(sc_copy, request->user.date, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_register(request);
	request_status_msg(REGISTER); 	
	return (SUCCESS);
} 

t_error_code server_request_unregister(int sc_copy, t_request *request, t_response *response)
{
	printf("UNREGISTER\n"); 
	if(ERROR == readLine(sc_copy, request->user.user_name, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(!db_user_exist(request->user.user_name))
	{
		response->status = USER_ERROR;
		return (SUCCESS); 
	}

	response->status = db_unregister(request->user.user_name);
	request_status_msg(UNREGISTER); 	
	return (SUCCESS);
} 

t_error_code server_request_connect(int sc_copy, t_request *request, t_response *response)
{
	if(ERROR == readLine(sc_copy, request->user.user_name, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(!db_user_exist(request->user.user_name))
	{
		response->status = USER_ERROR;
		request_status_msg(CONNECT); 
		return (SUCCESS); 
	}

	response->status = db_connect(request);
	request_status_msg(CONNECT); 	
	return (SUCCESS);
}

t_error_code server_request_disconnect(int sc_copy, t_request *request, t_response *response)
{
	if(ERROR == readLine(sc_copy, request->user.user_name, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	if(!db_user_exist(request->user.user_name))
	{
		response->status = USER_ERROR;
		return (SUCCESS); 
	}

	response->status = db_disconnect(request->user.user_name);
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