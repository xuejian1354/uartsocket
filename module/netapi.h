/*
 * netapi.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __NETAPI_H__
#define __NETAPI_H__

#include <services/globals.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	DE_UDP_SEND,
	DE_UDP_RECV,
	DE_TCP_ACCEPT,
	DE_TCP_SEND,
	DE_TCP_RECV,
	DE_TCP_RELEASE,
	DE_POST_SEND,
	DE_POST_RET
}de_print_t;

typedef struct
{
	int fd;
	struct sockaddr_in addr; 
	char *buf;
	int len;
}frhandler_arg_t;


frhandler_arg_t *get_frhandler_arg_alloc(int fd, struct sockaddr_in *addr, char *buf, int len);
void get_frhandler_arg_free(frhandler_arg_t *arg);

#ifdef TRANS_TCP_SERVER
int get_stcp_fd();
int socket_tcp_server_init(int port);
int socket_tcp_server_accept(int fd);
#endif
#if defined(TRANS_TCP_SERVER) || defined(UART_COMMBY_SOCKET)
int socket_tcp_server_recv(int fd);
void socket_tcp_server_send(frhandler_arg_t *arg, char *data, int len);
#endif

#ifdef TRANS_TCP_CLIENT
int get_ctcp_fd();
int socket_tcp_client_connect(int port);
int socket_tcp_client_recv();
void socket_tcp_client_send(char *data, int len);
void socket_tcp_client_close();
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
int get_udp_fd();
int socket_udp_service_init(int port);
void socket_udp_sendto_with_ipaddr(char *ipaddr, char *data, int len);
void socket_udp_sendto(struct sockaddr_in *addr, char *data, int len);
int socket_udp_recvfrom();
#endif

void set_deuart_flag(uint8 flag);
int get_deuart_flag();
#ifdef DE_TRANS_UDP_STREAM_LOG
void delog_udp_sendto(char *data, int len);
#endif

void trans_data_show(de_print_t deprint,
				struct sockaddr_in *addr, char *data, int len);

void enable_datalog_atime();

#ifdef __cplusplus
}
#endif

#endif  // __NETAPI_H__
