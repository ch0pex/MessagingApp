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

	if(ERROR == readLine(sc_copy, request->user->full_name, MAX_SIZE)) // Se recibe el nombre completo del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6 

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE)) // Se recibe el alias del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6

	if(ERROR == readLine(sc_copy, request->user->date, MAX_SIZE)) // Se recibe la fecha de nacimiento del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6 

	response->status = db_register(request); // Se almacena el usuario en la base de datos
	sprintf(nl_response, "%d", response->status); // Se convierte el entero a string

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) // Se envia el response status al cliente
		return(SEND_ERROR);  // ERROR CODE 5
	
	request_status_msg(REGISTER); // Macro para imprimir el mensaje de registro, utiliza repsonse->status para saber si ha sido correcto o no
	return (SUCCESS); // ERROR CODE 0
} 

/*************************************************************
 * Function: Funcion que procesa la peticion de baja de un usuario,
 * recibe el nombre de usuario y lo elimina de la base de datos
**************************************************************/
t_error_code server_request_unregister(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[2];

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE)) // Se recibe el alias del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6

	response->status = db_unregister(request->user->alias); // Se elimina el usuario de la base de datos
	sprintf(nl_response, "%d", response->status);

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))  // Se envia el response status al cliente
		return(SEND_ERROR); // ERROR CODE 5
	
	request_status_msg(UNREGISTER);  // Macro para imprimir el mensaje de baja 

	return (SUCCESS); // ERROR CODE 0
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

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE)) // Se recibe el alias del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6
	
	if(ERROR == readLine(sc_copy, request->user->port, MAX_SIZE)) // Se recibe el puerto del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6
	
	response->status = db_connect(request); // Se modifica el usuario con el nuevo estado, puerto e ip
	sprintf(nl_response, "%d", response->status);

	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) // Se envia el response status al cliente
		return(SEND_ERROR); // ERROR CODE 5
	
	request_status_msg(CONNECT); 

	if(response->status != RESPONSE_OK) // Si repsponse status es FAIL o USER_ALREADY_CONNECTED no se envian los mensajes pendientes
		return (SUCCESS); // ERROR CODE 0 

	db_get_messages(request->user->alias, &response->user->messages);
	if(response->user->messages.count == 0)
		return (SUCCESS); // ERROR CODE 0
	while(++i < response->user->messages.count)
	{
		// Si hay un fallo de conexión con el hilo de escucha el servidor asume que el cliente se ha desconectado y cmabia su estado a DISCONNECTED en la base de datos
		if(SUCCESS != server_request_connect_to_user(&sd, request->user->alias))
		{
			while(++i < response->user->messages.count)
				free(response->user->messages.array[i]); // Se liberan los mensajes pendientes
			free(response->user->messages.array); // Se liberan los mensajes pendientes
			return (server_request_disconnect(sc_copy, request, response)); // Se cambia el estado del usuario a DISCONNECTED y se borra la ip y el puerto de conexion
		}
		// Si la conexión es correcta se envian los mensajes pendientes
		server_request_send_message(sd, request->user->alias, (t_message *) response->user->messages.array[i]);
		free(response->user->messages.array[i]); // Se liberan los mensajes pendientes de memoria
		close(sd);
	}
	free(response->user->messages.array); // Se liberan los mensajes pendientes de memoria
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

	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE)) // Se recibe el alias del usuario 
		return (RECEIVE_ERROR); // ERROR CODE 6

	response->status = db_disconnect(request, response); // Se modifica el usuario con el nuevo estado, puerto e ip
	sprintf(nl_response, "%d", response->status);
	
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))  // Se envia el response status al cliente
		return(SEND_ERROR);  // ERROR CODE 5

	request_status_msg(DISCONNECT); 
	return (SUCCESS); // ERROR CODE 0
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
	
	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE)) // Se recibe el alias del usuario
		return (RECEIVE_ERROR); // ERROR CODE 6

	if(ERROR == readLine(sc_copy, request->send_to, MAX_SIZE)) // Se recibe el alias del usuario al que se envia el mensaje
		return (RECEIVE_ERROR);	 // ERROR CODE 6
	
	if(ERROR == readLine(sc_copy, request->message_content, MAX_SIZE)) // Se recibe el contenido del mensaje 
		return (RECEIVE_ERROR); // ERROR CODE 6

	strcpy(send_message.from, request->user->alias);
	strcpy(send_message.text, request->message_content);
	response->status = db_store_message(&send_message, request->send_to); // Se almacena el mensaje en la base de datos
	sprintf(nl_response, "%d", response->status);
	if(ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))  // Se envia el response status al cliente
		return(SEND_ERROR);  // ERROR CODE 5

	if(response->status != RESPONSE_OK) // Si repsponse status es FAIL o USER_NOT_FOUND no se envia el mensaje
	{
		sprintf(nl_response, "%d", response->status);
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
			return(SEND_ERROR); // ERROR CODE 5
		return (SUCCESS); // ERROR CODE 0
	}
	
	sprintf(nl_message_id, "%u", send_message.id); // Se envia el id del mensaje al cliente
	if(ERROR == sendMessage(sc_copy, (char *) &nl_message_id, strlen(nl_message_id) + 1))  // Se envia el response status al cliente
		return(SEND_ERROR);

	if(!db_user_is_connected(request->send_to)) // Si el usuario no esta conectado se almacena el mensaje en la base de datos y no se envia
	{
		printf("SEND MESSAGE %u FROM %s TO %s STORED\n", send_message.id, send_message.from, request->send_to);
		return (SUCCESS);
	}
	
	// Si hay un fallo de conexión con el hilo de escucha el servidor asume que el cliente se ha desconectado y cmabia su estado a DISCONNECTED en la base de datos
	if(SUCCESS != server_request_connect_to_user(&sd, request->send_to)) 
	{
		response->status = RESPONSE_FAIL; 
		sprintf(nl_response, "%d", response->status);
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) 
			return(SEND_ERROR); 
		requeste->user->alias = request->send_to; // Se cambia el alias del usuario para que se desconecte 
		return (server_request_disconnect(sc_copy, request, response)); // 
	}

	// Si el usuario esta conectado se envia el mensaje
	if (SUCCESS == server_request_send_message(sd, request->send_to, &send_message))
	{
		printf("SEND MESSAGE %u FROM %s TO %s\n", send_message.id, send_message.from, request->send_to);
		db_delete_message(request->send_to, send_message.id); // Si se ha enviado el mensaje con existo se borra de la base de datos
	}
	close(sd);
	return (SUCCESS); // ERROR CODE 0
} 

/**********************************************************************
 * Function: Funcion que procesa la petición para comporbar los usuarios 
 * conectados en el servidor, recibe el nombre de usuario y comprueba que 
 * este registrado y conectado para llevar acabo la ooperacion.
 **********************************************************************/
t_error_code server_request_connected_users(int sc_copy, t_request *request, t_response *response)
{
	char nl_response[MAX_SIZE];
	
	if(ERROR == readLine(sc_copy, request->user->alias, MAX_SIZE)) // Se recibe el alias del usuario
		return (RECEIVE_ERROR);  // ERROR CODE 6

	response->status = db_connected_users(request, response); // Se alamcena en response->connected_users los usuarios conectados
	sprintf(nl_response, "%d", response->status);
	
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))  // Se envia el response status al cliente
		return(SEND_ERROR);
	
	if(response->status != RESPONSE_OK) // Si repsponse status es FAIL o USER_NOT_FOUND no se envian mas datos 
	{
		request_status_msg(CONNECTED_USERS);
		return (SUCCESS);
	}
	
	sprintf(nl_response, "%u", response->connected_users.count);
	if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1))  // Se envia el numero de usuarios conectados
		return(SEND_ERROR);

	/*Se envian todos los usuarios registrados mientras se libera la memoria del dinamic array que los almacena*/
	while(response->connected_users.count--)
	{
		sprintf(nl_response, "%s", (char *) response->connected_users.array[response->connected_users.count]);
		if (SEND_ERROR == sendMessage(sc_copy, (char *) &nl_response, strlen(nl_response) + 1)) // Se envia el nombre de usuario
		{ //Si hay un erro al enviar se libera la memoria y se devuelve el error
			while(response->connected_users.count--)
				free(response->connected_users.array[response->connected_users.count]);
			free(response->connected_users.array);
			return(SEND_ERROR);
		} 
		free(response->connected_users.array[response->connected_users.count]); // Se libera la memoria del nombre de usuario
	}
	free(response->connected_users.array); // Se libera la memoria del dinamic array
	request_status_msg(CONNECTED_USERS);
	return (SUCCESS);
}

/* ************************************************************************* *
*  Funcion: Esta función crea una conexión con el usuario al que se le quiere
*  si no se conecta devuelve el error interono CONNECT_ERROR
*************************************************************************** */
static t_error_code server_request_connect_to_user(int *sd, char *user)
{
	struct sockaddr_in server_addr;
	struct hostent *send_to_hp;  
	char send_to_ip[16];
	char send_to_port[6];

	memset(send_to_ip, 0, 16); 
	memset(send_to_port, 0, 6);

	db_get_user_field(user, PORT_FIELD, send_to_port); // Se obtiene el puerto del usuario
	db_get_user_field(user, IP_FIELD, send_to_ip); // Se obtiene la ip del usuario

	*sd = socket(AF_INET, SOCK_STREAM, 0); 
	if (*sd == 1)  // Si no se puede crear el socket se devuelve el error interno SOCKET_ERROR
		return (SOCKET_ERROR); // ERROR CODE 1

	if ((send_to_hp = gethostbyname(send_to_ip)) == NULL) // Se obtiene la ip del usuario, si no se puede se devuelve el error interno HOSTNAME_ERROR
		return (HOSTNAME_ERROR); // ERROR CODE 7

	memcpy(&server_addr.sin_addr, send_to_hp->h_addr, send_to_hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(send_to_port));

	if (connect(*sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) // Se conecta con el usuario, si no se puede se devuelve el error interno CONNECT_ERROR
		return (CONNECT_ERROR); // ERROR CODE 8
	
	return (SUCCESS); // ERROR CODE 0
}

/* ************************************************************************* *
*  Funcion: Esta función envia un mensaje al usuario por su hilo de escucha
*************************************************************************** */  
static t_error_code server_request_send_message(int sc_copy, char *user, t_message *message)
{
	char nl_op[MAX_SIZE]; 
	char nl_id[11];

	strcpy(nl_op, "SEND_MESSAGE");
	sprintf(nl_id, "%u", message->id);

	if(ERROR == sendMessage(sc_copy, (char *) &nl_op, strlen(nl_op) + 1))  // Se envia el codigo de operacion al hilo de escucha
		return(SEND_ERROR); // ERROR CODE 5
	
	if(ERROR == sendMessage(sc_copy, (char *) &message->from, strlen(message->from) + 1)) // Se envia el remitente 
		return(SEND_ERROR); // ERROR CODE 5 

	if(ERROR == sendMessage(sc_copy, (char *) &nl_id, strlen(nl_id) + 1)) // Se envia el id del mensaje
		return(SEND_ERROR); // ERROR CODE 5

	if(ERROR == sendMessage(sc_copy, (char *) &message->text, strlen(message->text) + 1)) // Se envia el texto del mensaje
		return(SEND_ERROR); // ERROR CODE 5

	printf("SEND MESSAGE %u FROM %s - %s\n", message->id, message->from,message->text); 
	db_delete_message(user, message->id); // Si se ha enviado el mensaje con existo se borra de la base de datos
	return(SUCCESS);

}