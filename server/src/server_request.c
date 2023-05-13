/* ********************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para tratar las  */
/*	peticiones recibidas por el servidor    							  */
/* ********************************************************************** */

#include "server_request.h"

t_error_code server_request_register(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];

	if(ERROR == readLine(sc_copy, request->user.full_name, MAX_SIZE))
		return (RECEIVE_ERROR); 

	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(ERROR == readLine(sc_copy, request->user.date, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_register(request);
	sprintf(nl_response, "%d", response->status);

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR); 
	
	request_status_msg(REGISTER); 
	return (SUCCESS);
} 

t_error_code server_request_unregister(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];

	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_unregister(request->user.alias);
	sprintf(nl_response, "%d", response->status);

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))  
		return(SEND_ERROR); 
	
	request_status_msg(UNREGISTER); 

	return (SUCCESS);
} 

t_error_code server_request_connect(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];


	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	if(ERROR == readLine(sc_copy, request->user.port, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	response->status = db_connect(request, response);
	sprintf(nl_response, "%d", response->status);

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR); 
	
	request_status_msg(CONNECT); 

	// pillar los mensajes pendientes, si existen bucle enviando los mensajes

	return (SUCCESS);			
}

t_error_code server_request_disconnect(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];

	if(ERROR == readLine(sc_copy, request->user.alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_disconnect(request, response);
	sprintf(nl_response, "%d", response->status);
	
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR); 

	request_status_msg(DISCONNECT); 
	return (SUCCESS);
}

t_error_code server_request_send_message(int sc_copy, t_request *request, t_response *response)
{
	
	return (SUCCESS);
} 

t_error_code server_request_connected_users(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[MAX_SIZE];
	
	response->status = db_connected_users(request, response);
	sprintf(nl_response, "%d", response->status);
	
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR);
	
	if(response->status != OK)
	{
		request_status_msg(CONNECTED_USERS);
		return (SUCCESS);
	}
	
	sprintf(nl_response, "%ld", response->connected_users.count);
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR);

	while(response->connected_users.count--)
	{
		sprintf(nl_response, "%s", response->connected_users.array[response->connected_users.count]);
		printf("Enviando: %s ; %ld\n", nl_response, strlen(nl_response));
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
			return(SEND_ERROR);
	}
	return (SUCCESS);
}