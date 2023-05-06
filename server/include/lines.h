#ifndef _LINES_H
#define _LINES_H

#include <unistd.h>

#include "error_code.h"

t_error_code sendMessage(int socket, char *buffer, int len);
t_error_code recvMessage(int socket, char *buffer, int len);
ssize_t readLine(int fd, void *buffer, size_t n);

#endif