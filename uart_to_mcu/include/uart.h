#ifndef __UART_H
#define __UART_H

#include <fcntl.h>
#include <unistd.h>
#include <termio.h>
#include <stdio.h> 

int uart_init(char *uart_path);

#endif
