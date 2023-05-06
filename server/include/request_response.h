#ifndef REQUEST_RESPONSE_H
#define REQUEST_RESPONSE_H

#include <stdbool.h>

typedef enum{
	OK = 0,
	FAIL, 
	USER_ERROR
} t_response_status; 

typedef enum{
	REGISTER = 0,
	UNREGISTER, 
	CONNECT,
	DISCONNECT,
	SEND_MESSAGE,
	CONNECTED_USERS
} t_request_code; 

typedef struct{
	char *full_name; 
	char *user_name; 
	char *date;
	bool state; 
	char *IP; 
	int port; 
	char **messages; 
} t_user_data; 

typedef struct {
	int op;
	t_user_data user; 
} t_request;

typedef struct{
	t_response_status status;
	t_user_data user; 
} t_response; 



#endif 