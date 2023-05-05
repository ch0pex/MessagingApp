#ifndef REQUEST_RESPONSE_H
#define REQUEST_RESPONSE_H

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

typedef struct {
	int op; 
} t_request;

typedef struct{
	t_response_status status;
} t_response; 

#endif 