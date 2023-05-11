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
t_response_status db_connect(t_request *request, t_response *response); 
t_response_status db_disconnect(char *username); 
t_response_status db_send_message(t_request *request); 
t_response_status db_connected_users(); 
FILE* db_open_user(char *user); 
t_error_code db_get_field(FILE *file, char *field, char *value);
t_error_code db_set_field(FILE *file, char *field, char *value);

#endif 