#ifndef REQUEST_RESPONSE_H
#define REQUEST_RESPONSE_H

#include <stdbool.h>

#define MAX_SIZE 256
#define REGISTER   "REGISTER"
#define UNREGISTER "UNREGISTER"
#define CONNECT "CONNECT"
#define DISCONNECT "DISCONNECT"
#define SEND_MESSAGE "SEND_MESSAGE"
#define CONNECTED_USERS "CONNECTEDUSERS"


typedef enum{
	OK = 0,
	USER_ERROR,
	FAIL
} t_response_status; 

/*
typedef enum{
	REGISTER = 0,
	UNREGISTER, 
	CONNECT,
	DISCONNECT,
	SEND_MESSAGE,
	CONNECTED_USERS
} t_request_code; 
*/
typedef struct{
	char full_name[MAX_SIZE]; 
	char alias[MAX_SIZE]; 
	char date[MAX_SIZE];
	char state[MAX_SIZE];
	char ip[MAX_SIZE]; 
	char port[6]; 
} t_user_data; 

typedef struct {

	char op[MAX_SIZE]; 
	t_user_data user; 
} t_request;

typedef struct{
	t_response_status status;
	t_user_data user; 
} t_response; 



#endif 