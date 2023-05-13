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

#define FULL_NAME_FIELD "Nombre completo"
#define ALIAS_FIELD "Alias"
#define DATE_FIELD "Fecha de nacimiento"
#define STATE_FIELD "Estado"
#define IP_FIELD "IP"
#define PORT_FIELD "Puerto"
#define MESSAGES_FIELD "Mensaje"

t_response_status db_register(t_request *request); 
t_response_status db_unregister(char *username); 
t_response_status db_connect(t_request *request); 
t_response_status db_disconnect(t_request *request, t_response *response); 
t_response_status db_send_message(t_request *request); 
t_response_status db_connected_users(t_request *request, t_response *response); 

FILE* db_open_file(char *user, char *mode);
bool db_user_exists(char *user); 
bool db_user_is_connected(char *user);

t_error_code db_get_field(char *user, char *field, char *value);
t_error_code db_set_field(char *user, char *field, char *value);
t_error_code db_get_messages(FILE *file, char *messages);

#endif 