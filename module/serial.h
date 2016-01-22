/*
 * serial.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <services/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SERIAL_SUPPORT

#define SERIAL_MAX_LEN 128

int serial_open(char *dev);
int set_serial_params(int fd, 
	uint32 speed, uint8 databit, uint8 stopbit, uint8 parity);
#ifdef UART_COMMBY_SOCKET
int get_uart_refd();
int get_reser_fd();
int get_reser_accept(int fd);
#endif
int serial_init(char *dev);
int serial_write(char *data, int datalen);
void *uart_read_func(void *p);

#endif

#ifdef __cplusplus
}
#endif

#endif //__SERIAL_H__
