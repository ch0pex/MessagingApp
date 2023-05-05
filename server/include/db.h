#ifndef DB_H
#define DB_H

#include <dirent.h>

#include "request_response.h"

t_response_status db_register(); 
t_response_status db_unregister(); 
t_response_status db_connect(); 
t_response_status db_disconnect(); 
t_response_status db_send_message(); 
t_response_status db_connected_users(); 

#endif