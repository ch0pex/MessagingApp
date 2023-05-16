
#include <stdio.h>
#include "db.h"
#include "request_response.h"

/*int main(void)
{
    t_message msg; 
    char user[MAX_SIZE];
    t_request *request;
    t_response res;
    request = (t_request *) malloc(sizeof(t_request));
    request->user = (t_user_data *) malloc(sizeof(t_user_data));
    

    strcpy(request->user->alias, "paco");
    strcpy(request->user->full_name, "paco");
    strcpy(request->user->date, "23/32/8123");
    db_register(request); 
    strcpy(request->user->alias, "juan");
    strcpy(request->user->full_name, "juan");
    strcpy(request->user->date, "23/32/8123");
    db_register(request);
    strcpy(request->user->alias, "pedro");
    strcpy(request->user->full_name, "pedro");
    strcpy(request->user->date, "23/32/8123");
    db_register(request);
    strcpy(request->user->alias, "maria");
    strcpy(request->user->full_name, "maria");
    strcpy(request->user->date, "23/32/8123");
    db_register(request);
    strcpy(user, "paco");
    strcpy(msg.from, "maria"); 
    strcpy(msg.text, "Hola");
    db_store_message(&msg, user);
    strcpy(msg.from, "pedro");
    strcpy(msg.text, "Hola");
    db_store_message(&msg, user);
    strcpy(msg.from, "juan");
    strcpy(msg.text, "Hola");
    db_store_message(&msg, user);
    strcpy(user, "juan");
    strcpy(msg.from, "maria");
    strcpy(msg.text, "Hola");
    db_store_message(&msg, user);
    strcpy(msg.from, "pedro");
    strcpy(msg.text, " feo");
    db_store_message(&msg, user); 

    free(request->user);
    free(request);
    return (0);
}
*/