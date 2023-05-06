#ifndef DB_H
#define DB_H

#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>

#include "request_response.h"

t_response_status db_register(t_request *request); 
t_response_status db_unregister(char *username); 
t_response_status db_connect(t_request *request); 
t_response_status db_disconnect(char *username); 
t_response_status db_send_message(t_request *request); 
t_response_status db_connected_users(); 
bool db_user_exist(char *user); 
#endif