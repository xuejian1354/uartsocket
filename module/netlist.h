/*
 * netlist.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __NETLIST_H__
#define __NETLIST_H__

#include <services/globals.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	COMM_TCLIENT,
	RESER_TCLIENT,
}tclient_type_t;

typedef struct TCPConn
{
	int fd;
	tclient_type_t tclient;
	struct sockaddr_in client_addr;
	struct TCPConn *next;
}tcp_conn_t;

typedef struct TCPConnList
{
	tcp_conn_t *p_head;
	int num;
	const int max_size;
}tcp_conn_list_t;

tcp_conn_list_t *get_tcp_conn_list();
int addto_tcpconn_list(tcp_conn_t *list);
tcp_conn_t *queryfrom_tcpconn_list(int fd);
tcp_conn_t *queryfrom_tcpconn_list_with_ipaddr(char *ipaddr);
int delfrom_tcpconn_list(int fd);

#ifdef __cplusplus
}
#endif

#endif  // __NETLIST_H__