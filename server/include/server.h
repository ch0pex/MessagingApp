#ifndef SOCKET_H 
#define SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "error_code.h"
#include "server_request.h"
#include "lines.h"

// Estructura con todo lo necesario para crear un server con sockets

typedef struct {
	int sc; 
	struct sockaddr_in  client_addr;
} t_client_info; 


typedef struct {
    int sd, val;
	t_client_info client_info;
	struct sockaddr_in server_addr;	
	socklen_t size;
	pthread_t thid;
	pthread_attr_t t_attr;

} t_server; 

t_error_code server_init(t_server *server, char *port); 
t_error_code server_socket_create(t_server *server);
t_error_code server_socket_config(t_server *server, int port); 
void server_thread_config(t_server *server); 
void server_loop(t_server *server); 
void server_treat_request(void *sc);

#endif 