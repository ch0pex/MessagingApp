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
void sigint_handler() // Cuando se recibe un SIGINT se cierra el socket y se sale del programa
{
	close(sd_copy); 
	exit(SUCCESS); 
} 

/**********************************************************
 * Function: Inicializacion del servidor
 *********************************************************/
t_error_code server_init(t_server *server, char* port)
{
	t_error_code err; 

	err = server_socket_create(server); // Creacion del socket
	if (err != SUCCESS)
		return (err); // ERROR CODE correspondiente devuelto por la funcion de creacion

	err = server_socket_config(server, atoi(port)); // Configuracion del socket
	if (err != SUCCESS)
		return (err); // ERROR CODE correspondiente devuelto por la funcion de configuracion

	printf("init server %s:%d\n", inet_ntoa(server->server_addr.sin_addr), atoi(port));
	server_thread_config(server); // Configuracion de los threads
	return (SUCCESS); // ERROR CODE 0
}

/**********************************************************
 * Function: Crea el socket descriptor 
 *********************************************************/
t_error_code server_socket_create(t_server *server) 
{
	if((server->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Creacion del socket
		return (SOCKET_ERROR); // ERROR CODE 1
	sd_copy = server->sd; 
	return (SUCCESS); // ERROR CODE 0
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


	err = bind(server->sd, (const struct sockaddr *)&server->server_addr, sizeof(server->server_addr)); // Bind del socket
	if (err != SUCCESS) // Si hay error en el bind
		return (BIND_ERROR); // ERROR CODE 2

	err = listen(server->sd, SOMAXCONN); // Listen del socket
	if (err != SUCCESS)  // Si hay error en el listen
		return(LISTEN_ERROR); // ERROR CODE 3

	server->size = sizeof(server->server_addr); 
	return (SUCCESS); // ERROR CODE 0
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
	while (1){ // Bucle infinito de escucha de conexiones
		server->client_info.sc = accept(server->sd, (struct sockaddr *)&server->client_info.client_addr, (socklen_t *)&server->size); // Aceptacion de la conexion
		if (server->client_info.sc == ERROR){ // Si hay error en el accept
			error_code_print_msg(ACCEPT_ERROR); // Se imprime el error en el servidor 
			continue; 
		}
		if (pthread_create(&server->thid, &server->t_attr, (void *)server_treat_request, (void *)&server->client_info.sc) == 0) // Creacion del hilo para tratar la peticion
		{
			pthread_mutex_lock(&mutex_sc);
			while (sc_not_copied)
				pthread_cond_wait(&cond_sc, &mutex_sc);
			sc_not_copied = true;
			pthread_mutex_unlock(&mutex_sc);
		}                     
	}
	// Se cierra y se termina la ejecucion del programa en sigint_handler
}

/**********************************************************
 * Function: Trata las peticiones de los clientes de forma
 * concurrente en otro hilo.
 *********************************************************/
void server_treat_request(void *sc)
{
	t_request request; 
	t_response response;
	t_client_info client_info_copy;
	int err; 

	request.user = (t_user_data *) calloc(1, sizeof(t_user_data)); // Reserva de memoria para la informacion del usuario (evitamos sobrecargar la pila)
	response.user = (t_user_data *) calloc(1, sizeof(t_user_data)); // Reserva de memoria para la informacion del usuario (evitamos sobrecargar la pila)

	pthread_mutex_lock(&mutex_sc);
	client_info_copy = (*(t_client_info*) sc);
	sc_not_copied = false;
	pthread_cond_signal(&cond_sc);
	pthread_mutex_unlock(&mutex_sc);
	
	err = readLine(client_info_copy.sc, (char *) &request.op, MAX_SIZE); // Lectura del codigo de operacion 
	if (err == ERROR) // Si la lectura da error no se procesa la peticion, se libera la memoria, se muestra el mensaje de error y se cierra la conexion y el thread 
	{
		free(request.user);
		free(response.user);
		error_code_print_msg(err); 
		close(client_info_copy.sc); 
		pthread_exit(0); 
	}
	// Se procesa la peticion en funcion del codigo de operacion recibido 
	if(strcmp(REGISTER, request.op) == SUCCESS)
		err = server_request_register(client_info_copy.sc, &request, &response); // Se procesa la peticion de registro
	else if(strcmp(UNREGISTER, request.op) == SUCCESS)
		err = server_request_unregister(client_info_copy.sc, &request, &response); // Se procesa la peticion de baja
	else if(strcmp(CONNECT, request.op) == SUCCESS){
		inet_ntop(AF_INET, &(client_info_copy.client_addr.sin_addr), request.user->ip, INET_ADDRSTRLEN); // IP del cliente
		err = server_request_connect(client_info_copy.sc, &request, &response); // Se procesa la peticion de conexion
	}	
	else if(strcmp(DISCONNECT, request.op) == SUCCESS)
		err = server_request_disconnect(client_info_copy.sc, &request, &response); // Se procesa la peticion de desconexion
	else if(strcmp(SEND_MESSAGE, request.op) == SUCCESS)
		err = server_request_process_message(client_info_copy.sc, &request, &response); // Se procesa la peticion de envio de mensaje de un usuario a otro
	else if(strcmp(CONNECTED_USERS, request.op) == SUCCESS)	
		err = server_request_connected_users(client_info_copy.sc, &request, &response); // Se procesa la peticion de listado de usuarios conectados
	
	/* Todas las operaciones devuelven un ERROR CODE, este codigo de error es interno al servidor y no es enviado al cliente*/
	if (err != SUCCESS) // Si huboo algun error en el procesamiento de la peticion
	{ 
		free(request.user); // Se libera la memoria de la informacion del usuario
		free(response.user); // Se libera la memoria de la informacion del usuario 
		error_code_print_msg(err); // Se imprime el error en el servidor
		close(client_info_copy.sc); 
		pthread_exit(err); // Se cierra el thread con el codigo de error
	}

	free(request.user); // Se libera la memoria de la informacion del usuario
	free(response.user); // Se libera la memoria de la informacion del usuario
	close(client_info_copy.sc);
	pthread_exit(0);
}


