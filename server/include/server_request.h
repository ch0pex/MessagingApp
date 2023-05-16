#ifndef SERVER_REQUEST_H 
#define SERVER_REQUEST_H

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "lines.h"
#include "error_code.h"
#include "db.h"
#include "request_response.h"

#define request_status_msg(op) do { \
	if(response->status == RESPONSE_OK) \
		printf("%s %s OK\n", op, request->user->alias); \
	else \
		printf("%s %s FAIL\n", op, request->user->alias); \
} while(0) 

t_error_code server_request_register(int sc_copy, t_request *request, t_response *response); 
t_error_code server_request_unregister(int sc_copy, t_request *request, t_response *response); 
t_error_code server_request_connect(int sc_copy, t_request *request, t_response *response); 
t_error_code server_request_disconnect(int sc_copy, t_request *request, t_response *response);
t_error_code server_request_process_message(int sc_copy, t_request *request, t_response *response); 
t_error_code server_request_connected_users(int sc_copy, t_request *request, t_response *response);

#endif
