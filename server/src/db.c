/* ************************************************************************* */
/* Este fichero contiene las funciones necesarias para el manejo de ficheros */
/* donde se guardara todos los datos de los distintos usuarios               */ 
/* ************************************************************************* */

#include "db.h"

t_response_status db_register(t_request *request)
{
	DIR *db_dir = opendir("db");
	char *user_path;  
	FILE *user_data; 

	if(!db_dir) 
	{
		if(mkdir("db", 0755) < 0){
			closedir(db_dir); 
			return (FAIL); 
		}
	}

	if ((user_data = db_open_user(request->user.alias)) != NULL)
	{
		closedir(db_dir);
		return (USER_ERROR); 
	}

	user_path = (char *) malloc(strlen(request->user.alias) + 8); 
	sprintf(user_path, "db/%s.txt", request->user.alias ); 
	user_data = fopen(user_path, "w");
	free(user_path); 

	fprintf(user_data, 
	"Nombre completo: %s\nAlias: %s\nFecha de nacimiento: %s\nEstado: DISCONNECTED\nIP: \nPuerto: \nMensajes: \n",
	request->user.full_name,request->user.alias, request->user.date); 
	
	fclose(user_data); 
	closedir(db_dir); 
	return (OK); 
}

t_response_status db_unregister(char *username)
{
	char *user_path = (char *) malloc(strlen(username) + 8); 

	sprintf(user_path, "db/%s.txt",username); 
	if (remove(user_path) == ERROR)
	{
		free(user_path); 
		return (USER_ERROR);
	}
	free(user_path); 
	return (OK); 
}

t_response_status db_connect(t_request *request, t_response *response)
{
	FILE *user_data;

	user_data = db_open_user(request->user.alias);

	if (user_data == NULL)
		return (USER_ERROR); 
	
	db_get_field(user_data, STATE_FIELD, response->user.state);
	
	if(strcmp(response->user.state, "CONNECTED") == 0) 
	{
		fclose(user_data); 
		return (FAIL); 
	}
	
	db_set_field(user_data, STATE_FIELD, "CONNECTED");
	db_set_field(user_data, IP_FIELD, request->user.ip); 
	db_set_field(user_data, PORT_FIELD, request->user.port);
		
	fclose(user_data);
	return (OK); 
} 

t_response_status db_disconnect(t_request *request, t_response *response)
{
	FILE *user_data;

	user_data = db_open_user(request->user.alias);

	if (user_data == NULL)
		return (USER_ERROR); 

	db_get_field(user_data, STATE_FIELD, response->user.state);

	if(strcmp(response->user.state, "DISCONNECTED") == 0) // Si el estado es DISCONNECTED
	{
		fclose(user_data); 
		return (FAIL); 
	}

	db_set_field(user_data, STATE_FIELD, "DISCONNECTED");
	db_set_field(user_data, IP_FIELD, response->user.ip);
	db_set_field(user_data, PORT_FIELD, response->user.port);

	fclose(user_data);
	return (OK);
}

t_response_status db_send_message(t_request *request)
{
	return (OK); 
}

t_response_status db_connected_users(t_request *request, t_response *response)
{
	DIR *dir = opendir("db");
	FILE *user_data;
	struct dirent *ent;
	response->connected_users.count = 0; 
	response->connected_users.array = NULL;  


	user_data = db_open_user(request->user.alias);
	if(user_data == NULL)
		return (FAIL);
	
	db_get_field(user_data, STATE_FIELD, response->user.state);

	if (strcmp(response->user.state, "DISCONNECTED") == 0) // Si el estado es DISCONNECTED 
	{
		fclose(user_data); 
		return (USER_ERROR); 
	}

	fclose(user_data);


	while ((ent = readdir(dir)))
	{
		char user_name[MAX_SIZE];
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;

		strcpy(user_name, ent->d_name);
		user_name[strlen(user_name) - 4] = '\0';
		user_data = db_open_user(user_name);
		if (user_data == NULL)
			continue;

		db_get_field(user_data, STATE_FIELD, response->user.state);
		if(strcmp(response->user.state, "DISCONNECTED") == 0){
			fclose(user_data);
			continue;
		}
		
		db_get_field(user_data, ALIAS_FIELD, response->user.alias);
		printf("%s Longitud: %ld", response->user.alias, strlen(response->user.alias));
		response->connected_users.count++;
		response->connected_users.array = (char **) realloc(response->connected_users.array, response->connected_users.count * sizeof(char *));
		response->connected_users.array[response->connected_users.count - 1] = (char *) malloc(strlen(response->user.alias) + 1);
		strcpy(response->connected_users.array[response->connected_users.count - 1], response->user.alias);
		fclose(user_data);	
	}
	closedir(dir);
	return (OK);

}



/**********************************************************
 * Function: Devuelve el archivo con los datos del usuario 
 * si existe el usuario sino devuelve NULL
 *********************************************************/
FILE* db_open_user(char *user)
{
	char *user_path = (char *) malloc(strlen(user) + 8);  
	FILE* user_data; 

	sprintf(user_path, "db/%s.txt", user); 
	user_data = fopen(user_path, "r+"); 
	free(user_path); 

	return (user_data); 
}


t_error_code db_set_field(FILE *file, char *field, char *value)
{	
	char alias[MAX_SIZE];
	char full_name[MAX_SIZE];
	char date[MAX_SIZE];
	char state[MAX_SIZE];
	char ip[MAX_SIZE];
	char port[MAX_SIZE];
	char messages[MAX_SIZE];
	
	memset(messages, 0, MAX_SIZE); // Cuando esten implementados los mensajes camibar por la funcion que los obtiene
	
	fscanf(file, "Nombre completo: %[^\n]\nAlias: %[^\n]\nFecha de nacimiento: %[^\n]\nEstado: %[^\n]\nIP: %[^\n]\nPuerto: %[^\n]\n", 
	full_name, alias, date, state, ip, port);

	if(strcmp(field, "Nombre completo") == 0)
		strcpy(full_name, value);
	else if(strcmp(field, "Alias") == 0)
		strcpy(alias, value);
	else if(strcmp(field, "Fecha de nacimiento") == 0)
		strcpy(date, value);
	else if(strcmp(field, "Estado") == 0)
		strcpy(state, value);
	else if(strcmp(field, "IP") == 0)
		strcpy(ip, value);
	else if(strcmp(field, "Puerto") == 0)
		strcpy(port, value);

	db_clean_file(alias); // Limpiamos el archivo para volver a escribirlo
	fseek(file, 0, SEEK_SET);
	fprintf(file, "Nombre completo: %s\nAlias: %s\nFecha de nacimiento: %s\nEstado: %s\nIP: %s\nPuerto: %s\nMensajes: %s\n", 
	full_name, alias, date, state, ip, port, messages);
	return (SUCCESS);
}

t_error_code db_get_field(FILE *file, char *field, char *value)
{
	t_error_code err = ERROR;
	char *line = NULL; 
	size_t len = 0; 
	ssize_t read; 
	char *field_name = (char *) malloc(strlen(field) + 2); 
	sprintf(field_name, "%s:", field); 

	while ((read = getline(&line, &len, file)) != -1)
	{

		if (strstr(line, MESSAGES_FIELD) != NULL){
			break; 
		}
		if (strstr(line, field_name) != NULL)
		{
			strcpy(value, line + strlen(field_name) + 1); 
			value[strlen(value) - 1] = '\0';
			err = SUCCESS;
			break; 
		}
	}
	fseek(file, 0, SEEK_SET);
	free(field_name); 
	free(line); 
	return (err); 
}

t_error_code db_get_messages(FILE *file, char *messages)
{
	t_error_code err = ERROR;
	char *line = NULL; 
	size_t len = 0; 
	ssize_t read; 
	char *field_name = (char *) malloc(strlen(MESSAGES_FIELD) + 2); 
	sprintf(field_name, "%s:", MESSAGES_FIELD); 

	while ((read = getline(&line, &len, file)) != -1)
	{
		if (strstr(line, field_name) != NULL)
		{
			strcpy(messages, line + strlen(field_name) + 1); 
			err = SUCCESS;
			break; 
		}
	}
	fseek(file, 0, SEEK_SET);
	free(field_name); 
	free(line); 
	return (err); 
}

void db_clean_file(char *user)
{
	char *user_path = (char *) malloc(strlen(user) + 8);  
	FILE* user_data; 

	sprintf(user_path, "db/%s.txt", user); 
	user_data = fopen(user_path, "w"); 
	free(user_path); 
	fclose(user_data);
}