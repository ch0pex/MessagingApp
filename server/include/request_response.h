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

typedef struct{
	char full_name[MAX_SIZE]; 
	char alias[MAX_SIZE]; 
	char date[MAX_SIZE];
	char state[MAX_SIZE];
	char ip[MAX_SIZE]; 
	char port[6]; 
	char messages[MAX_SIZE]; // TODO: Cambiar a un array de strings
} t_user_data; 

typedef struct {
	char op[MAX_SIZE]; 
	t_user_data *user; 
} t_request;

typedef struct {
	size_t count;  
	char **array; 
} t_dynamic_array;

typedef struct{
	t_response_status status;
	t_user_data *user; 
	t_dynamic_array connected_users;
	t_dynamic_array messages;
} t_response; 

#endif 