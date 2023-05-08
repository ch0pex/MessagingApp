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

	user_path = (char *) malloc(strlen(request->user.alias + 8)); 
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

t_response_status db_connect(t_request *request)
{
	return (OK); 
} 

t_response_status db_disconnect(char *username)
{
	return (OK); 
}

t_response_status db_send_message(t_request *request)
{
	return (OK); 
}

t_response_status db_connected_users()
{
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
	user_data = fopen(user_path, "r"); 
	free(user_path); 

	return (user_data); 
}