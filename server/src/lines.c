#include <unistd.h>
#include <errno.h>
#include "lines.h"


/* ************************************************************************* *
*  Funcion: Esta funcion envia un mensaje a traves de un socket.              *
*************************************************************************** */
t_error_code sendMessage(int socket, char * buffer, int len)
{
	int r;
	int l = len;
		

	do {	
		r = write(socket, buffer, l);
		l = l -r;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (SEND_ERROR);   /* fail */
	else
		return(SUCCESS);	/* full length has been sent */
}

/* ************************************************************************* *
*  Funcion: Esta funcion recibe un mensaje a traves de un socket.             *
*************************************************************************** */
t_error_code recvMessage(int socket, char *buffer, int len)
{
	int r;
	int l = len;

	do {	
		r = read(socket, buffer, l);
		l = l -r ;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (RECEIVE_ERROR);   /* fallo */
	else
		return(SUCCESS);	/* full length has been receive */
}

/* ************************************************************************* *
*  Funcion: Esta funcion recibe un mensaje a traves de un socket.             *
*************************************************************************** */
ssize_t readLine(int fd, void *buffer, size_t n)
{
	ssize_t numRead;  /* num of bytes fetched by last read() */
	size_t totRead;	  /* total bytes read so far */
	char *buf;
	char ch;

	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return (-1); 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
    	numRead = read(fd, &ch, 1);	/* read a byte */

    	if (numRead == -1) {	
        	if (errno == EINTR)	/* interrupted -> restart read() */
           		continue;
        	else
				return -1;		/* some other error */
    	} else if (numRead == 0) {	/* EOF */
        	if (totRead == 0)	/* no byres read; return 0 */
            	return 0;
			else
            	break;
    	} else {			/* numRead must be 1 if we get here*/
    		if (ch == '\n')
        		break;
    		if (ch == '\0')
        		break;
    		if (totRead < n - 1) {		/* discard > (n-1) bytes */
				totRead++;
				*buf++ = ch; 
			}
		}	 
	}
	
	*buf = '\0';
    	return totRead;
}