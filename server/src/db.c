/* ************************************************************************* */
/* Este fichero contiene las funciones necesarias para el manejo de ficheros */
/* donde se guardara todos los datos de los distintos usuarios               */ 
/* ************************************************************************* */

#include "db.h"


/* ************************************************************************* *
*  Funcion: Esta función registra al usuairo en la base de datos, crea el fichero
*  del usuario y lo rellena con los datos del usuario iniciales
*  Devuevle: RESPONSE_STATUS 
*************************************************************************** */
t_response_status db_register(const t_request *request)
{
	DIR *db_dir = opendir("db");
	FILE *user_data; 
	char *user_path;  

	if(!db_dir) // Si no existe el directorio db se crea 
	{
		if(mkdir("db", 0755) < 0){
			closedir(db_dir); 
			return (RESPONSE_FAIL); // response->status = 2
		}
	}

	if (db_user_exists(request->user->alias))  // Si el usuario ya existe se el status USER_ERROR = 1
	{
		closedir(db_dir);
		return (RESPONSE_USER_ERROR); // response->status = 1
	}

	user_path = (char *) malloc(strlen(request->user->alias) + 8); 
	sprintf(user_path, "db/%s.txt", request->user->alias ); 
	user_data = fopen(user_path, "w");
	free(user_path); 

	fprintf(user_data, 
	"Nombre completo: %s\nAlias: %s\nFecha de nacimiento: %s\nEstado: DISCONNECTED\nIP: Null\nPuerto: Null\nUltimo mensaje: 0\nMensajes:\n",
	request->user->full_name,request->user->alias, request->user->date); // Se rellena el fichero con los datos del usuario
	
	fclose(user_data); 
	closedir(db_dir); 
	return (RESPONSE_OK); 
}

/* ************************************************************************* *
*  Funcion: Esta función elemina al usuario de la base de datos, es decir 
*  elimina el fichero del usuario
*  Devuevle: RESPONSE_STATUS 
*************************************************************************** */
t_response_status db_unregister(const char *username)
{
	char *user_path = (char *) malloc(strlen(username) + 8); 

	sprintf(user_path, "db/%s.txt",username); 
	if (remove(user_path) == ERROR) // Si no se puede eliminar el fichero se devuelve USER_ERROR = 1
	{ 
		free(user_path); 
		return (RESPONSE_USER_ERROR); // response->status = 1
	}
	free(user_path); 
	return (RESPONSE_OK); //response->status = 0
}

/* ************************************************************************* *
*  Funcion: Esta función cambia en el fichero del usuario los datos para que 
* pase a estar conectado, es decir, cambia el estado a CONNECTED, la IP y el puerto
*************************************************************************** */
t_response_status db_connect(const t_request *request)
{
	if (!db_user_exists(request->user->alias)) // Si el usuario no existe se devuelve USER_ERROR = 1
		return (RESPONSE_USER_ERROR); // response->status = 1

	if(db_user_is_connected(request->user->alias)) // Si el estado es CONNECTED se devuelve FAIL = 2
		return (RESPONSE_FAIL); // response->status = 2
	
	db_set_user_field(request->user->alias, STATE_FIELD, "CONNECTED");
	db_set_user_field(request->user->alias, IP_FIELD, request->user->ip); 
	db_set_user_field(request->user->alias, PORT_FIELD, request->user->port);
		
	return (RESPONSE_OK); // response->status = 0
} 

/* ************************************************************************* *
*  Funcion: Esta función desconecta al usuario, es decir, cambia el estado a
*  DISCONNECTED, la IP y el puerto a Null
*  Devuevle: RESPONSE_STATUS 
*************************************************************************** */
t_response_status db_disconnect(const t_request *request, t_response *response)
{
	if (!db_user_exists(request->user->alias)) // Si el usuario no existe se devuelve USER_ERROR = 1
		return (RESPONSE_USER_ERROR); // response->status = 1

	if(!db_user_is_connected(request->user->alias)) // Si el estado es DISCONNECTED se devuelve FAIL = 2
		return (RESPONSE_FAIL); // response->status = 2

	/*Se vacian los campos ip y port por NULL*/
	sprintf(response->user->ip,"%s", "Null");
	sprintf(response->user->port,"%s", "Null");
	db_set_user_field(request->user->alias, STATE_FIELD, "DISCONNECTED"); 
	db_set_user_field(request->user->alias, IP_FIELD, response->user->ip);
	db_set_user_field(request->user->alias, PORT_FIELD, response->user->port);

	return (RESPONSE_OK); // response->status = 0
}

/* ************************************************************************* *
*  Funcion: Esta funcion almacena un mensaje en el fichero del usuario correspondiente
* a su vez actualiza el campo de ultimo mensaje del usuario
*  Devuevle: RESPONSE_STATUS 
*************************************************************************** */
t_response_status db_store_message(t_message *message, const char *send_to)
{
	FILE *user_data;
	char str_id[11]; 

	memset(str_id, 0, 11);

	if(!db_user_exists(send_to) || !db_user_exists(message->from)) // Si el remitente o el destinatario no existen se devuelve USER_ERROR = 1
		return (RESPONSE_USER_ERROR); // response->status = 1

	user_data = db_open_file(send_to, "r+");

	db_get_user_field(send_to, LAST_MESSAGE_FIELD, str_id); // Se obtiene el ultimo mensaje del usuario
	message->id = strtoul(str_id, NULL, 10);
	message->id++; // Se incrementa el id del mensaje
	sprintf(str_id, "%u", message->id);
	db_set_user_field(send_to, LAST_MESSAGE_FIELD, str_id);	// Se actualiza el ultimo mensaje del usuario

	fseek(user_data, 0, SEEK_END);
	fprintf(user_data, "%u %s - %s\n", message->id, message->from, message->text); // Se almacena el mensaje en el fichero del usuario
	fclose(user_data);	
	
	return (RESPONSE_OK); // response->status = 0
}

/* ************************************************************************* *
*  Funcion: Esta funcion obtiene los mensajes del usuario y los almacena en un
* array dinamico pasado por parametro
*  Devuevle: RESPONSE_STATUS 
*************************************************************************** */
t_response_status db_get_messages(const char *user, t_dynamic_array *messages)
{
	FILE *user_data; 
	t_message tmp_message; 

	user_data = db_open_file(user, "r");
	if(!user_data)
		return (RESPONSE_USER_ERROR); // response->status = 1
	fscanf(user_data,"Nombre completo: %*[^\n]\nAlias: %*[^\n]\nFecha de nacimiento: %*[^\n]\nEstado: %*[^\n]\nIP: %*[^\n]\nPuerto: %*[^\n]\nUltimo mensaje: %*u\nMensajes:\n"); // Se saltan los campos del usuario hasta los mensajes
	/*Se obtienen los todos los mensajes del usuairo mediante scanf y se almacenan en el array dinamico pasado por parametro*/
	while(fscanf(user_data, "%u %[^ -] - %[^\n]\n", &tmp_message.id, tmp_message.from, tmp_message.text) != EOF) 
	{
		messages->count++; // Se incrementa el contador de mensajes del array dinamico 
		messages->array = (void **) realloc(messages->array, messages->count * sizeof(t_message *)); // Se reserva memoria para el puntero del nuevo mensaje
		messages->array[messages->count - 1] = (t_message *) calloc(1, sizeof(t_message)); // Se reserva memoria para el nuevo mensaje
		memcpy(messages->array[messages->count - 1], &tmp_message, sizeof(t_message)); // Se copia el mensaje temporal en el array dinamico
	}
	// La memoria del dinamic array se libera cuando se lee en server_request_connect
	fclose(user_data); 
	return(RESPONSE_OK); // response->status = 0
}

/* ************************************************************************* *
*  Funcion: Esta función registra devuelve en un array dinamico los usuarios
*  connectados en el sistema
*  Devuevle: RESPONSE_STATUS 
*************************************************************************** */
t_response_status db_connected_users(const t_request *request, t_response *response)
{
	DIR *dir = opendir("db");
	struct dirent *ent;

	response->connected_users.count = 0; 
	response->connected_users.array = NULL;  

	if (!db_user_is_connected(request->user->alias))  // Si el usuario no esta conectado se devuelve FAIL = 2
	{
		closedir(dir);
		return (RESPONSE_USER_ERROR); // response->status = 1
	}

	while ((ent = readdir(dir))) // Se recorre el directorio de usuarios
	{
		char user_name[MAX_SIZE];
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;

		strcpy(user_name, ent->d_name);
		user_name[strlen(user_name) - 4] = '\0';

		if(!db_user_is_connected(user_name)) // Si el usuario no esta conectado se salta y no se almacena en el array dinamico
			continue;
		
		db_get_user_field(user_name, ALIAS_FIELD, response->user->alias); // Se obtiene el alias del usuario
		response->connected_users.count++; // Se incrementa el contador de usuarios conectados
		response->connected_users.array = (void **) realloc(response->connected_users.array, response->connected_users.count * sizeof(char *)); // Se reserva memoria para el puntero del nuevo usuario
		response->connected_users.array[response->connected_users.count - 1] = (char *) malloc(strlen(response->user->alias) + 1); // Se reserva memoria para el nuevo usuario
		strcpy(response->connected_users.array[response->connected_users.count - 1], response->user->alias); // Se copia el usuario en el array dinamico	 
	}
	// La memoria del dinamic array se libera cuando se lee en server_request_connected_users
	closedir(dir);
	return (RESPONSE_OK); // response->status = 0
}

/* ************************************************************************* *
*  Funcion: Esta función abre el fichero del usuario y devuelve el descriptor
*  Devuelve: FILE *
*************************************************************************** */
FILE* db_open_file(const char *user, const char *mode)
{
	FILE *user_data; 
	char *user_path = (char *) malloc(strlen(user) + 8);  

	sprintf(user_path, "db/%s.txt", user); 
	user_data = fopen(user_path, mode); 
	free(user_path); 
	return (user_data); 
}

/**********************************************************
 * Function: Esta funcion comprueba si existe el usuario
 * Devuelve: bool
 *********************************************************/
bool db_user_exists(const char *user)
{
	FILE *user_file = db_open_file(user, "r");
	if(user_file== NULL)
		return (false);
	
	fclose(user_file);
	return (true); 
}

/**********************************************************
 * Function: Esta funcion comprueba si el usuario esta conectado
 * Devuelve: bool
 *********************************************************/
bool db_user_is_connected(const char *user) 
{
	char state[MAX_SIZE];

	memset(state, 0, MAX_SIZE);	
	db_get_user_field(user, STATE_FIELD, state);
	
	return (!strcmp(state, "CONNECTED")); 
}

/**********************************************************
 * Function: Cambia el valor del campo pasado por parametro
 * Devuelve: ERROR_CODE 
 *********************************************************/
t_error_code db_set_user_field(const char *user, const char *field, const char *value)
{	
	FILE *file;  
	t_user_data *tmp_data; 
	char *raw_messages; // Todos los mensajes que tenga el usuario en una cadena de texto para preservarlos al modificar cualquier campo
	long int file_size; 

	tmp_data =  (t_user_data *) calloc(1, sizeof(t_user_data));
	file = db_open_file(user, "r+");
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	if(file == NULL)
	{
		free(tmp_data);
		return (ERROR); // No se ha podido abrir el fichero ERROR INTERNO = -1
	}
	
	fscanf(file, "Nombre completo: %[^\n]\nAlias: %[^\n]\nFecha de nacimiento: %[^\n]\nEstado: %[^\n]\nIP:  %[^\n]\nPuerto: %[^\n]\nUltimo mensaje: %u\nMensajes:\n", 
	tmp_data->full_name, tmp_data->alias, tmp_data->date, tmp_data->state, tmp_data->ip, tmp_data->port, &tmp_data->last_message); // Se lee el fichero y se almacena en la estructura temporal
	
	raw_messages = (char *) calloc(1, file_size - ftell(file) + 1); // Se reserva memoria para todos los mensajes del usuario
	fread(raw_messages, 1, file_size - ftell(file), file); // Se leen todos los mensajes del usuario para preservarlos

	fclose(db_open_file(tmp_data->alias, "w"));  // Limpiamos el archivo para volver a escribirlo

	/*En función del campo pasado por parametro se cambia el valor*/
	if(strcmp(field, FULL_NAME_FIELD) == 0)
		strcpy(tmp_data->full_name, value);
	else if(strcmp(field, ALIAS_FIELD) == 0)
		strcpy(tmp_data->alias, value);
	else if(strcmp(field, DATE_FIELD) == 0)
		strcpy(tmp_data->date, value);
	else if(strcmp(field, STATE_FIELD) == 0)
		strcpy(tmp_data->state, value);
	else if(strcmp(field, IP_FIELD) == 0)
		strcpy(tmp_data->ip, value);
	else if(strcmp(field, PORT_FIELD) == 0)
		strcpy(tmp_data->port, value);
	else if(strcmp(field, LAST_MESSAGE_FIELD) == 0)
		tmp_data->last_message = strtoul(value, NULL, 10);

	fseek(file, 0, SEEK_SET);
	fprintf(file, "Nombre completo: %s\nAlias: %s\nFecha de nacimiento: %s\nEstado: %s\nIP: %s\nPuerto: %s\nUltimo mensaje: %u\nMensajes:\n", 
	tmp_data->full_name, tmp_data->alias, tmp_data->date, tmp_data->state, tmp_data->ip, tmp_data->port, tmp_data->last_message); // Se escribe la estructura temporal en el fichero
	fwrite(raw_messages, 1, strlen(raw_messages), file); // Se escriben los mensajes del usuario
	free(raw_messages);
	free(tmp_data); 
	fclose(file);
	return (SUCCESS); // Se devuelve el codigo de error SUCCESS = 0
}

/**********************************************************
 * Function: Obtiene el valor del campo pasado por parametro
 * y lo almacena en value
 * Devuelve: ERROR_CODE 
 *********************************************************/
t_error_code db_get_user_field(const char *user, const char *field, char *value)
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
	return (err); // Se devuelve el codigo de error, si se encuentra el campo se devuelve SUCCESS = 0 sino ERROR = -1
}

/**********************************************************
* Function:Esta funcion elimina el mensaje con el id pasado 
* por parametro
* Devuelve: ERROR_CODE 
*********************************************************/
t_error_code db_delete_message(const char *user, const unsigned int id)
{
	FILE *user_data;
	long int fields_size; 
	t_dynamic_array messages;
	char *raw_fields;
	t_message *ptr_message; 
	unsigned int i = -1;
	t_error_code err = ERROR;
	
	messages.array = NULL;
	messages.count = 0;
	db_get_messages(user, &messages); // Obtenemos todos los mensajes del usuario y los almacenamos en el array dinamico

	user_data = db_open_file(user, "r+"); // Abrimos el fichero del usuario en modo lectura y escritura
	if(user_data == NULL) // Si no se ha podido abrir el fichero se devuelve ERROR
		return (ERROR); // ERROR_CODE = -1
	
	fscanf(user_data,"Nombre completo: %*[^\n]\nAlias: %*[^\n]\nFecha de nacimiento: %*[^\n]\nEstado: %*[^\n]\nIP:  %*[^\n]\nPuerto: %*[^\n]\nUltimo mensaje: %*u\nMensajes:\n"); 
	fields_size = ftell(user_data);
	raw_fields = (char *) calloc(1, fields_size + 1); // Se reserva memoria para todos los campos del usuario
	rewind(user_data); 
	fread(raw_fields, 1, fields_size, user_data); // Se leen todos los campos del usuario para preservarlos
	fclose(db_open_file(user, "w")); // Limpiamos el archivo para volver a escribirlo
	rewind(user_data); // Se posiciona el puntero al principio del fichero
	fwrite(raw_fields, 1, fields_size, user_data); // Se escriben los campos del usuario

	while (++i < messages.count) 
	{

		ptr_message = (t_message *) messages.array[i];
		if (ptr_message->id == id) // Si el id del mensaje coincide con el pasado por parametro no se escribe en el fichero
		{
			err = SUCCESS;
			free(ptr_message);
			continue;
		}
		fprintf(user_data, "%u %s - %s\n", ptr_message->id, ptr_message->from, ptr_message->text); 
		free(ptr_message); // A la vez que se lee del array dinamico se libera la memoria de cada mensaje
	}

	free(messages.array); // Se libera la memoria del array dinamico
	fclose(user_data);
	free(raw_fields);
	return (err); 
}


