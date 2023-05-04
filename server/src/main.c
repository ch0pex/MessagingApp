#include "main.h"



int main(int argc, char **argv)
{
	t_error_code err;
	t_server server; 

	err = parser_arg_check(argc, argv); 

	if (err == SUCCESS)
	{
		err = server_init(&server, argv[1]); 
		if (err == SUCCESS)
		{
			err = server_loop(&server);
		}
	}
	
	return (error_code_print_msg(err));
}


