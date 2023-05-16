#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <stdio.h>


//Mensajes de error 
#define INTERNAL_ERROR_MSG "Error interno\n"
#define SOCKET_ERROR_MSG "Error en el socket\n"
#define BIND_ERROR_MSG "Error en bind\n"
#define LISTEN_ERROR_MSG "Error en listen\n"
#define ACCEPT_ERROR_MSG "Error en accept\n"
#define EXT_NO_ARG_MSG "Error, es necesario pasar el puerto como argumento para lanzar el servidor\n"
#define EXT_TO_MANY_ARG_MSG "Error, demasiados argumentos, solo indique el puerto\n"
#define EXT_BAD_PORT_MSG  "Error, puerto no valido (1024-65535)\n"
#define SEND_ERROR_MSG "Error al recibir el mensaje...\n"
#define RECEIVE_ERROR_MSG "Error al recibir el mensaje...\n"
#define HOST_ERROR_MSG "Error en gethostbyname\n"
#define CONNECT_ERROR_MSG "Error al conectar con el servidor...\n"

//Errores del servidor
typedef enum  {
	ERROR = -1, 
	SUCCESS = 0,
	SOCKET_ERROR,
	BIND_ERROR,
	LISTEN_ERROR,
	ACCEPT_ERROR,
	SEND_ERROR,
	RECEIVE_ERROR,
	HOSTNAME_ERROR,
	CONNECT_ERROR,
	EXT_TO_MANY_ARG,
	EXT_NO_ARG,
	EXT_BAD_PORT

} t_error_code; 


t_error_code error_code_print_msg( t_error_code err);


#endif 

