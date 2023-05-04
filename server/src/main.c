#include "main.h"

pthread_mutex_t mutex_sc;
int sc_not_copied = true;
pthread_cond_t cond_sc;

void tratar_peticion(void *sc)
{
	
}



int main(int argc, char **argv)
{
	t_error_code err;
	struct sockaddr_in server_addr,  client_addr;	
	socklen_t size;
    int sd, sc, val;
	pthread_t thid;
	pthread_attr_t t_attr;

	err = parser_arg_check(argc, argv); 
	if (err != SUCCESS)
		return (error_code_print_msg(err)); 
	
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (error_code_print_msg(SOCKET_ERROR)); 

 	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port        = htons(atoi(argv[1]));

	err = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err != SUCCESS)
		return (error_code_print_msg(BIND_ERROR)); 

	err = listen(sd, SOMAXCONN);
	if (err != SUCCESS) 
		return(error_code_print_msg(LISTEN_ERROR)); 

	pthread_mutex_init(&mutex_sc, NULL);
	pthread_cond_init(&cond_sc, NULL);
	pthread_attr_init(&t_attr);
	pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

	while (1){
		sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);


		if (sc != SUCCESS) {
			return (error_code_print_msg(ACCEPT_ERROR)); 
		}


		if (pthread_create(&thid, &t_attr, (void *)tratar_peticion, (void *)&sc) == 0) {
			pthread_mutex_lock(&mutex_sc);
			while (sc_not_copied)
				pthread_cond_wait(&cond_sc, &mutex_sc);
			sc_not_copied = true;
			pthread_mutex_unlock(&mutex_sc);
		}                     
	}
	close (sd);
	return (err);
}


