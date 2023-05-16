#ifndef REQUEST_RESPONSE_H
#define REQUEST_RESPONSE_H

#include <stdbool.h>

#define MAX_SIZE 256
#define MAX_MESSAGES (unsigned int) -1
#define REGISTER   "REGISTER"
#define UNREGISTER "UNREGISTER"
#define CONNECT "CONNECT"
#define DISCONNECT "DISCONNECT"
#define SEND_MESSAGE "SEND"
#define CONNECTED_USERS "CONNECTEDUSERS"




typedef enum{
	RESPONSE_OK = 0,
	RESPONSE_USER_ERROR,
	RESPONSE_FAIL
} t_response_status; 

typedef struct{
	unsigned int id; 
	char from[MAX_SIZE]; 
	char text[MAX_SIZE]; 
} t_message;

typedef struct {
	unsigned int count;  
	void **array; 
} t_dynamic_array;

typedef struct{
	char full_name[MAX_SIZE]; 
	char alias[MAX_SIZE]; 
	char date[MAX_SIZE];
	char state[MAX_SIZE];
	char ip[16]; // [XXX.XXX.XXX.XXX]
	char port[6]; 
	unsigned int last_message; 
	t_dynamic_array messages; 
	//t_dynamic_array messages; // TODO: Cambiar a un array de strings
} t_user_data; 

typedef struct {
	char op[MAX_SIZE]; 
	t_user_data *user;
	char send_to[MAX_SIZE];
	char message_content[MAX_SIZE]; 
} t_request;

typedef struct{
	t_response_status status;
	t_user_data *user; 
	t_dynamic_array connected_users;
} t_response; 

#endif 