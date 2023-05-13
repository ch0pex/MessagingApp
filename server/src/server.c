/* **************************************************************************** */
/*  Este fichero contiene todas las funciones necesarias para crear, configurar */
/*  y ejecutar el servidor                                                      */                                              
/* **************************************************************************** */

#include "server.h"

/*--------------Variables globales sevidor---------------*/
pthread_mutex_t mutex_sc;
int sc_not_copied = true;
pthread_cond_t cond_sc;


/*------------Manejador de CTR + C (SIGINT)---------------*/
int sd_copy = 0;
void sigint_handler()
{
	printf("Entro aqui\n"); 
	close(sd_copy); 
	exit(SUCCESS); 
} 


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

	printf("init server %s:%d\n", inet_ntoa(server->server_addr.sin_addr), ntohs(server->server_addr.sin_port));
	fflush(stdout); 

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
	sd_copy = server->sd; 
	return (SUCCESS); 
}

/**********************************************************
 * Function: Configuracion del socket, bind y listen
 *********************************************************/
t_error_code server_socket_config(t_server *server, int port)
{
	t_error_code err;

	server->val = 1; 
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

	server->size = sizeof(server->server_addr); 
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
void server_loop(t_server *server) 
{
	signal(SIGINT, sigint_handler); //Configuración de la señal SIGINT
	while (1){
		server->client_info.sc = accept(server->sd, (struct sockaddr *)&server->client_info.client_addr, (socklen_t *)&server->size);
		if (server->client_info.sc == ERROR){
			error_code_print_msg(ACCEPT_ERROR); 
			continue; 
		}
		if (pthread_create(&server->thid, &server->t_attr, (void *)server_treat_request, (void *)&server->client_info.sc) == 0) {
			pthread_mutex_lock(&mutex_sc);
			while (sc_not_copied)
				pthread_cond_wait(&cond_sc, &mutex_sc);
			sc_not_copied = true;
			pthread_mutex_unlock(&mutex_sc);
		}                     
	}
	// Se cierra y se termina la ejecucion del programa en sigint_handler
	//close(server->sd) 
	
}

void server_treat_request(void *sc)
{
	t_request request; 
	t_response response;
	t_client_info client_info_copy;
	int err; 


	pthread_mutex_lock(&mutex_sc);
	client_info_copy = (*(t_client_info*) sc);
	sc_not_copied = false;
	pthread_cond_signal(&cond_sc);
	pthread_mutex_unlock(&mutex_sc);
	
	err = readLine(client_info_copy.sc, (char *) &request.op, MAX_SIZE);
	if (err == ERROR)
	{
		error_code_print_msg(err); 
		close(client_info_copy.sc); 
		pthread_exit(0); 
	}
	
	if(strcmp(REGISTER, request.op) == SUCCESS)
		err = server_request_register(client_info_copy.sc, &request, &response);
	else if(strcmp(UNREGISTER, request.op) == SUCCESS)
		err = server_request_unregister(client_info_copy.sc, &request, &response);
	else if(strcmp(CONNECT, request.op) == SUCCESS){
		inet_ntop(AF_INET, &(client_info_copy.client_addr.sin_addr), request.user.ip, INET_ADDRSTRLEN);
		err = server_request_connect(client_info_copy.sc, &request, &response);
	}
	else if(strcmp(DISCONNECT, request.op) == SUCCESS)
		err = server_request_disconnect(client_info_copy.sc, &request, &response);
	else if(strcmp(SEND_MESSAGE, request.op) == SUCCESS)
		err = server_request_send_message(client_info_copy.sc, &request, &response);
	else if(strcmp(CONNECTED_USERS, request.op) == SUCCESS)	
		err = server_request_connected_users(client_info_copy.sc, &request, &response);
	
	if (err != SUCCESS)
	{
		error_code_print_msg(err); 
		close(client_info_copy.sc); 
		pthread_exit(0); 
	}

	close(client_info_copy.sc);
	pthread_exit(0);
}