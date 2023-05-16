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
#define LAST_MESSAGE_FIELD "Ultimo mensaje"
#define MESSAGES_FIELD "Mensaje"


t_response_status db_register(const t_request *request); 
t_response_status db_unregister(const char *username); 
t_response_status db_connect(const t_request *request); 
t_response_status db_disconnect(const t_request *request, t_response *response); 
t_response_status db_store_message(t_message *message, const char *send_to); 
t_response_status db_get_messages(const char *user, t_dynamic_array *messages);
t_response_status db_connected_users(const t_request *request, t_response *response); 

FILE* db_open_file(const char *user, const char *mode);
bool db_user_exists(const char *user); 
bool db_user_is_connected(const char *user);

t_error_code db_get_user_field(const char *user, const char *field, char *value);
t_error_code db_set_user_field(const char *user, const char *field, const char *value);
t_error_code db_get_last_message(const char *user, t_message *message);
t_error_code db_delete_message(const char *user, const unsigned int id);    

#endif 