/* **************************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para crear, configurar */
/*  y ejecutar el servidor                                                      */                                              
/* **************************************************************************** */

#include "server.h"

/*--------------Variables globales sevidor---------------*/
pthread_mutex_t mutex_sc;
int sc_not_copied = true;
pthread_cond_t cond_sc;


/**********************************************************
 * Function: Inicializacion del servidor
 *********************************************************/
t_error_code server_init(t_server *server, char* port)
{
	t_error_code err; 

	err = server_socket_create(server);
	if (err != SUCCESS)
		return (err);  

	err = server_socket_config(server, atoi(port));
	if (err != SUCCESS)
		return (err); 

	server_thread_config(server); 
	return (SUCCESS); 
}

/**********************************************************
 * Function: Crea el socket descriptor 
 *********************************************************/
t_error_code server_socket_create(t_server *server) 
{
	if((server->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (SOCKET_ERROR); 
	return (SUCCESS); 
}

/**********************************************************
 * Function: Configuracion del socket, bind y listen
 *********************************************************/
t_error_code server_socket_config(t_server *server, int port)
{
	t_error_code err;

	setsockopt(server->sd, SOL_SOCKET, SO_REUSEADDR, (char *) &server->val, sizeof(int));
	bzero((char *)&server->server_addr, sizeof(server->server_addr));
	server->server_addr.sin_family      = AF_INET;
	server->server_addr.sin_addr.s_addr = INADDR_ANY;
	server->server_addr.sin_port        = htons(port);

	err = bind(server->sd, (const struct sockaddr *)&server->server_addr, sizeof(server->server_addr));
	if (err != SUCCESS)
		return (BIND_ERROR);

	err = listen(server->sd, SOMAXCONN);
	if (err != SUCCESS) 
		return(LISTEN_ERROR); 

	return (SUCCESS); 
}

/**********************************************************
 * Function: Configuracion thread attr, mutex y cond
 *********************************************************/
void server_thread_config(t_server *server)
{
	pthread_mutex_init(&mutex_sc, NULL);
	pthread_cond_init(&cond_sc, NULL);
	pthread_attr_init(&server->t_attr);
	pthread_attr_setdetachstate(&server->t_attr, PTHREAD_CREATE_DETACHED);
}

/**********************************************************
 * Function: Bucle principal del servidor concurrente
 * multihilo, en el se esperan conxiones infinitamente 
 * hasta que se reciba un SIGNINT
 *********************************************************/
t_error_code server_loop(t_server *server) 
{

	while (1){
		server->sc = accept(server->sd, (struct sockaddr *)&server->client_addr, (socklen_t *)&server->size);
		if (server->sc != SUCCESS) {
			return (ACCEPT_ERROR); 
		}

		if (pthread_create(&server->thid, &server->t_attr, (void *)server_treat_request, (void *)&server->sc) == 0) {
			pthread_mutex_lock(&mutex_sc);
			while (sc_not_copied)
				pthread_cond_wait(&cond_sc, &mutex_sc);
			sc_not_copied = true;
			pthread_mutex_unlock(&mutex_sc);
		}                     
	}
	close (server->sd);
	return (SUCCESS); 
}

void server_treat_request(void *sc)
{
	t_request request; 
	t_response response;
	int sc_copy, err; 


	pthread_mutex_lock(&mutex_sc);
	sc_copy = (*(int*) sc);
	sc_not_copied = false;
	pthread_cond_signal(&cond_sc);
	pthread_mutex_unlock(&mutex_sc);
	
	err = recvMessage(sc_copy, (char *) &request.op, sizeof(char));

	switch(request.op) {
    case REGISTER:			err = server_request_register(sc_copy, &request, &response); break;
    case UNREGISTER:		err = server_request_unregister(sc_copy, &request, &response); break;
    case CONNECT: 			err = server_request_connect(sc_copy, &request, &response); break;
    case DISCONNECT:		err = server_request_disconnect(sc_copy, &request, &response); break;
    case SEND_MESSAGE:		err = server_request_send_message(sc_copy, &request, &response);break;
    case CONNECTED_USERS:	err = server_request_connected_users(sc_copy, &request, &response); break;
	}

	if (err != SUCCESS)
	{
		error_code_print_msg(err); 
		close(sc_copy); 
		pthread_exit(0); 
	}

	response.status = htonl(response.status); 
	err = sendMessage(sc_copy, (char *) &response.status, sizeof(int32_t));
	if (err == SEND_ERROR) 
		error_code_print_msg(SEND_ERROR); 
	
	close(sc_copy);
	pthread_exit(0);
}