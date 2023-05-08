#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>


#include "request_response.h"
#include "error_code.h"

t_response_status db_register(t_request *request); 
t_response_status db_unregister(char *username); 
t_response_status db_connect(t_request *request); 
t_response_status db_disconnect(char *username); 
t_response_status db_send_message(t_request *request); 
t_response_status db_connected_users(); 
FILE* db_open_user(char *user); 
#endif