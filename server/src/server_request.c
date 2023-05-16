/* ********************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para tratar las  */
/*	peticiones recibidas por el servidor    							  */
/* ********************************************************************** */

#include "server_request.h"

static t_error_code server_request_connect_to_user(int* sd, char *user); 
static t_error_code server_request_send_message(int sc_copy, char *user, t_message *message);	

/*************************************************************
 * Function: Funcion que procesa la peticion de registro, 
 * recibe todos los mensajes necesarios por parte del cliente
 * para despues almacenarlo con la funcion db_register
 ************************************************************/
t_error_code server_request_register(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];

	if(ERROR == readLine(sc_copy, request->user->full_name, MAX_SIZE))
		return (RECEIVE_ERROR); 

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(ERROR == readLine(sc_copy, request->user->date, MAX_SIZE))
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

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_unregister(request->user->alias);
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
	int sd;
	unsigned int i;

	
	response->user->messages.count = 0;
	response->user->messages.array = NULL;
	i = -1; 

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	if(ERROR == readLine(sc_copy, request->user->port, MAX_SIZE))
		return (RECEIVE_ERROR);
	
	response->status = db_connect(request);
	sprintf(nl_response, "%d", response->status);

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR); 
	
	request_status_msg(CONNECT); 

	if(response->status != RESPONSE_OK) // Si repsponse status es FAIL o USER_ALREADY_CONNECTED no se envian los mensajes pendientes
		return (SUCCESS);

	db_get_messages(request->user->alias, &response->user->messages);
	if(response->user->messages.count == 0)
		return (SUCCESS);
	while(++i < response->user->messages.count)
	{
		if(SUCCESS != server_request_connect_to_user(&sd, request->user->alias))
		{
			while(++i < response->user->messages.count)
				free(response->user->messages.array[i]);
			free(response->user->messages.array);
			return(SUCCESS); //return (server_request_disconnect(sc_copy, request, response)); // Si hay un fallo de conexión se desconecta al usuario de la base datos
		}
		server_request_send_message(sd, request->user->alias, (t_message *) response->user->messages.array[i]);
		free(response->user->messages.array[i]); 
		close(sd);
	}
	free(response->user->messages.array);
	close(sd);
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

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_disconnect(request, response);
	sprintf(nl_response, "%d", response->status);
	
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR); 

	request_status_msg(DISCONNECT); 
	return (SUCCESS);
}

/**********************************************************************
 * Function: Función donde se procesa la petición de envio de un mensaje
 * si el usuario al que se envia esta conectado se le envia directamente
 * si no se almacena en la base de datos para que cuando se conecte se
 * le envie.
 **********************************************************************/
t_error_code server_request_process_message(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];
	char nl_message_id[11];
	t_message send_message;
	int sd; 
	


	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	if(ERROR == readLine(sc_copy, request->send_to, MAX_SIZE))
		return (RECEIVE_ERROR);	
	
	if(ERROR == readLine(sc_copy, request->message_content, MAX_SIZE))
		return (RECEIVE_ERROR);

	strcpy(send_message.from, request->user->alias);
	strcpy(send_message.text, request->message_content);
	response->status = db_store_message(&send_message, request->send_to);
	sprintf(nl_response, "%d", response->status);
	if(ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR); 

	if(response->status != RESPONSE_OK)
	{
		sprintf(nl_response, "%d", response->status);
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
			return(SEND_ERROR); 
		return (SUCCESS);
	}
	
	sprintf(nl_message_id, "%u", send_message.id);
	if(ERROR == sendMessage(sc_copy, (char *) &nl_message_id, strlen(nl_message_id) + 1)) 
		return(SEND_ERROR);

	if(!db_user_is_connected(request->send_to))
	{
		printf("SEND MESSAGE %u FROM %s TO %s STORED\n", send_message.id, send_message.from, request->send_to);
		return (SUCCESS);
	}
	
	if(SUCCESS != server_request_connect_to_user(&sd, request->send_to))
	{
		response->status = RESPONSE_FAIL; 
		sprintf(nl_response, "%d", response->status);
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
			return(SEND_ERROR); 
		return (SUCCESS);
	}

	if (SUCCESS == server_request_send_message(sd, request->send_to, &send_message))
	{
		printf("SEND MESSAGE %u FROM %s TO %s\n", send_message.id, send_message.from, request->send_to);
		db_delete_message(request->send_to, send_message.id); // Si se ha enviado el mensaje con existo se borra de la base de datos
	}
	close(sd);
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
	
	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE))
		return (RECEIVE_ERROR);

	response->status = db_connected_users(request, response);
	sprintf(nl_response, "%d", response->status);
	
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR);
	
	if(response->status != RESPONSE_OK)
	{
		request_status_msg(CONNECTED_USERS);
		return (SUCCESS);
	}
	
	sprintf(nl_response, "%u", response->connected_users.count);
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
		return(SEND_ERROR);

	while(response->connected_users.count--)
	{
		sprintf(nl_response, "%s", (char *) response->connected_users.array[response->connected_users.count]);
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))
		{ //Si hay un erro al enviar se libera la memoria y se devuelve el error
			while(response->connected_users.count--)
				free(response->connected_users.array[response->connected_users.count]);
			free(response->connected_users.array);
			return(SEND_ERROR);
		} 
			
		free(response->connected_users.array[response->connected_users.count]);
	}
	free(response->connected_users.array);
	request_status_msg(CONNECTED_USERS);
	return (SUCCESS);
}



static t_error_code server_request_connect_to_user(int *sd, char *user)
{
	struct sockaddr_in server_addr;
	struct hostent *send_to_hp;  
	char send_to_ip[16];
	char send_to_port[6];

	memset(send_to_ip, 0, 16); 
	memset(send_to_port, 0, 6);

	db_get_user_field(user, PORT_FIELD, send_to_port);
	db_get_user_field(user, IP_FIELD, send_to_ip);

	*sd = socket(AF_INET, SOCK_STREAM, 0); 
	if (*sd == 1) 
		return (SOCKET_ERROR);

	if ((send_to_hp = gethostbyname(send_to_ip)) == NULL)
		return (HOSTNAME_ERROR);

	memcpy(&server_addr.sin_addr, send_to_hp->h_addr, send_to_hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(send_to_port));

	if (connect(*sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		return (CONNECT_ERROR);
	
	return (SUCCESS);
}

static t_error_code server_request_send_message(int sc_copy, char *user, t_message *message)
{
	char nl_op[MAX_SIZE]; 
	char nl_id[11];

	strcpy(nl_op, "SEND_MESSAGE");
	sprintf(nl_id, "%u", message->id);



	if(ERROR == sendMessage(sc_copy, (char *) &nl_op, strlen(nl_op) + 1))
		return(SEND_ERROR);
	
	if(ERROR == sendMessage(sc_copy, (char *) &message->from, strlen(message->from) + 1))
		return(SEND_ERROR);

	if(ERROR == sendMessage(sc_copy, (char *) &nl_id, strlen(nl_id) + 1))
		return(SEND_ERROR);

	if(ERROR == sendMessage(sc_copy, (char *) &message->text, strlen(message->text) + 1))
		return(SEND_ERROR);

	printf("SEND MESSAGE %u FROM %s - %s\n", message->id, message->from,message->text); 
	db_delete_message(user, message->id); // Si se ha enviado el mensaje con existo se borra de la base de datos
	return(SUCCESS);

}