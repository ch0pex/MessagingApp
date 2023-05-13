/* ********************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para tratar las  */
/*	peticiones recibidas por el servidor    							  */
/* ********************************************************************** */

#include "server_request.h"

/*************************************************************
 * Function: Funcion que procesa la peticion de registro, 
 * recibe todos los mensajes necesarios por parte del cliente
 * para despues almacenarlo con la funcion db_register
 ************************************************************/
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


/*************************************************************
 * Function: Funcion que procesa la peticion de baja de un usuario,
 * recibe el nombre de usuario y lo elimina de la base de datos
**************************************************************/
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

/**********************************************************************
 * Function: Funcion que procesa la peticion de conexion de un usuario,
 * recibe el nombre de usuario y el puerto donde estara escuchando. 
 * Despues almacena en la base datos su nuevo estado el puerto y la ip
 * con la funcion db_connect. Si el usuario tenia mensajes pendientes
 * se los envia.
 **********************************************************************/
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

/**********************************************************************
 * Function: Funcion que procesa la peticion de desconexion de un usuario,
 * recibe el nombre de usuario, cambia su estado a DISCONNECTED y borra 
 * la ip y el puerto de conexion. 
 **********************************************************************/
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

/**********************************************************************
 * Function:
 **********************************************************************/
t_error_code server_request_send_message(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[MAX_SIZE];



	return (SUCCESS);
} 

/**********************************************************************
 * Function: Funcion que procesa la petición para comporbar los usuarios 
 * conectados en el servidor, recibe el nombre de usuario y comprueba que 
 * este registrado y conectado para llevar acabo la ooperacion.
 **********************************************************************/
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
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
			return(SEND_ERROR);
		free(response->connected_users.array[response->connected_users.count]);
	}
	free(response->connected_users.array);
	request_status_msg(CONNECTED_USERS);
	return (SUCCESS);
}