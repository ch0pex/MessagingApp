/* ************************************************************************* */
/* Este fichero contiene las funciones necesarias para el manejo de ficheros */
/* donde se guardara todos los datos de los distintos usuarios               */ 
/* ************************************************************************* */

#include "db.h"

t_response_status db_register(t_request *request)
{
	DIR *db_dir = opendir("db");
	FILE *user_data; 
	char *user_path;  

	if(!db_dir) 
	{
		if(mkdir("db", 0755) < 0){
			closedir(db_dir); 
			return (FAIL); 
		}
	}

	if (db_user_exists(request->user->alias)) 
	{
		closedir(db_dir);
		return (USER_ERROR); 
	}

	user_path = (char *) malloc(strlen(request->user->alias) + 8); 
	sprintf(user_path, "db/%s.txt", request->user->alias ); 
	user_data = fopen(user_path, "w");
	free(user_path); 

	fprintf(user_data, 
	"Nombre completo: %s\nAlias: %s\nFecha de nacimiento: %s\nEstado: DISCONNECTED\nIP: \nPuerto: \nMensajes: \n",
	request->user->full_name,request->user->alias, request->user->date); 
	
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

t_response_status db_connect(t_request *request)
{
	if (!db_user_exists(request->user->alias))
		return (USER_ERROR); 

	
	if(db_user_is_connected(request->user->alias))
		return (FAIL); 
	
	db_set_field(request->user->alias, STATE_FIELD, "CONNECTED");
	db_set_field(request->user->alias, IP_FIELD, request->user->ip); 
	db_set_field(request->user->alias, PORT_FIELD, request->user->port);
		
	return (OK); 
} 

t_response_status db_disconnect(t_request *request, t_response *response)
{

	if (!db_user_exists(request->user->alias))
		return (USER_ERROR); 

	if(!db_user_is_connected(request->user->alias)) // Si el estado es DISCONNECTED
		return (FAIL); 
	
	db_set_field(request->user->alias, STATE_FIELD, "DISCONNECTED");
	db_set_field(request->user->alias, IP_FIELD, response->user->ip);
	db_set_field(request->user->alias, PORT_FIELD, response->user->port);

	return (OK);
}

t_response_status db_send_message(t_request *request)
{
	return (OK); 
}

t_response_status db_connected_users(t_request *request, t_response *response)
{
	DIR *dir = opendir("db");
	struct dirent *ent;

	response->connected_users.count = 0; 
	response->connected_users.array = NULL;  

	if (!db_user_is_connected(request->user->alias))
	{
		closedir(dir);
		return (USER_ERROR);
	}

	while ((ent = readdir(dir)))
	{
		char user_name[MAX_SIZE];
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;

		strcpy(user_name, ent->d_name);
		user_name[strlen(user_name) - 4] = '\0';

		if(!db_user_is_connected(user_name))
			continue;
		
		db_get_field(user_name, ALIAS_FIELD, response->user->alias);
		response->connected_users.count++;
		response->connected_users.array = (char **) realloc(response->connected_users.array, response->connected_users.count * sizeof(char *));
		response->connected_users.array[response->connected_users.count - 1] = (char *) malloc(strlen(response->user->alias) + 1);
		strcpy(response->connected_users.array[response->connected_users.count - 1], response->user->alias);	
	}
	closedir(dir);
	return (OK);

}


FILE* db_open_file(char *user, char *mode)
{
	FILE *user_data; 
	char *user_path = (char *) malloc(strlen(user) + 8);  

	sprintf(user_path, "db/%s.txt", user); 
	user_data = fopen(user_path, mode); 
	free(user_path); 
	return (user_data); 
}

/**********************************************************
 * Function: Devuelve el archivo con los datos del usuario 
 * si existe el usuario sino devuelve NULL
 *********************************************************/
bool db_user_exists(char *user)
{
	FILE *user_file = db_open_file(user, "r");
	if(user_file== NULL)
		return (false);
	
	fclose(user_file);
	return (true); 
}

bool db_user_is_connected(char *user) 
{
	char state[MAX_SIZE];

	memset(state, 0, MAX_SIZE);	
	db_get_field(user, STATE_FIELD, state);
	
	return (!strcmp(state, "CONNECTED")); 
}

/**********************************************************
 * Function: Cambia el valor de un campo del archivo
 *********************************************************/
t_error_code db_set_field(char *user, char *field, char *value)
{	
	FILE *file; 
	t_user_data *tmp_data; 

	tmp_data = (t_user_data *) calloc(1,sizeof(t_user_data));
	 // Cuando esten implementados los mensajes camibar por la funcion que los obtiene
	
	file = db_open_file(user, "r+");
	if(file == NULL)
	{
		free(tmp_data);
		return (ERROR);
	}
	
	fscanf(file, "Nombre completo: %[^\n]\nAlias: %[^\n]\nFecha de nacimiento: %[^\n]\nEstado: %[^\n]\nIP: %[^\n]\nPuerto: %[^\n]\n", 
	tmp_data->full_name, tmp_data->alias, tmp_data->date, tmp_data->state, tmp_data->ip, tmp_data->port);

	if(strcmp(field, "Nombre completo") == 0)
		strcpy(tmp_data->full_name, value);
	else if(strcmp(field, "Alias") == 0)
		strcpy(tmp_data->alias, value);
	else if(strcmp(field, "Fecha de nacimiento") == 0)
		strcpy(tmp_data->date, value);
	else if(strcmp(field, "Estado") == 0)
		strcpy(tmp_data->state, value);
	else if(strcmp(field, "IP") == 0)
		strcpy(tmp_data->ip, value);
	else if(strcmp(field, "Puerto") == 0)
		strcpy(tmp_data->port, value);

	fclose(db_open_file(tmp_data->alias, "w")); // Limpiamos el archivo para volver a escribirlo
	fseek(file, 0, SEEK_SET);
	fprintf(file, "Nombre completo: %s\nAlias: %s\nFecha de nacimiento: %s\nEstado: %s\nIP: %s\nPuerto: %s\nMensajes: %s\n", 
	tmp_data->full_name, tmp_data->alias, tmp_data->date, tmp_data->state, tmp_data->ip, tmp_data->port, tmp_data->messages);
	fclose(file);
	free(tmp_data); 
	return (SUCCESS);
}

/**********************************************************
 * Function: Obtiene el valor de un campo del archivo
 *********************************************************/
t_error_code db_get_field(char *user, char *field, char *value)
{
	FILE *file;
	t_error_code err = ERROR;
	char *line = NULL; 
	size_t len = 0; 
	ssize_t read; 
	char *field_name = (char *) malloc(strlen(field) + 2); 

	sprintf(field_name, "%s:", field);
	file = db_open_file(user, "r");
	if(file == NULL)
	{
		free(field_name); 
		return (err);
	}

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
	fclose(file);
	return (err); 
}

t_error_code db_get_messages(FILE *file, char *messages)
{	/*
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
	free(line); */
	return (SUCCESS); 
}

t_error_code db_store_message(char *user, char *message)
{
	/*
	char *user_path = (char *) malloc(strlen(user) + 8);  
	FILE *user_data; 
	char messages[MAX_SIZE]; 

	sprintf(user_path, "db/%s.txt", user); 
	user_data = fopen(user_path, "r+"); 
	free(user_path); 

	db_get_messages(user_data, messages); 
	strcat(messages, message); 
	strcat(messages, "\n"); 
	db_set_field(user_data, MESSAGES_FIELD, messages); 

	fclose(user_data); */
	return (SUCCESS); 
}


