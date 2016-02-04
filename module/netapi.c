/*
 * netapi.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "netapi.h"
#include <module/netlist.h>
#include <services/corecomm.h>\

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TRANS_UDP_SERVICE)
static frhandler_arg_t t_arg;
static int udpfd;
static struct sockaddr_in m_addr;
#endif
#ifdef TRANS_TCP_SERVER
static int s_tcpfd;
#endif
#ifdef TRANS_TCP_CLIENT
static int c_tcpfd;
static struct sockaddr_in m_server_addr;
#endif

frhandler_arg_t *get_frhandler_arg_alloc(int fd, struct sockaddr_in *addr, char *buf, int len)
{
	if(len > MAXSIZE)
	{
		return NULL;
	}

	frhandler_arg_t *arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
	arg->buf = (char *)calloc(1, len);

	arg->fd = fd;

	if(addr != NULL)
	{
		memcpy(&arg->addr, addr, sizeof(struct sockaddr_in));
	}
	
	if(buf != NULL)
	{
		memcpy(arg->buf, buf, len);
		arg->len = len;
	}
	else
	{
		free(arg->buf);
		arg->buf = NULL;
		arg->len = 0;
	}

	return arg;
}

void get_frhandler_arg_free(frhandler_arg_t *arg)
{
	if(arg != NULL)
	{
		free(arg->buf);
		free(arg);
	}
}

#ifdef TRANS_TCP_SERVER
int get_stcp_fd()
{
	return s_tcpfd;
}

int socket_tcp_server_init(int port)
{
	struct sockaddr_in m_addr;
	
	if ((s_tcpfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("tcp socket fail");
		return -1;
	}
	
	m_addr.sin_family = PF_INET;
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.1.1");
	if (bind(s_tcpfd, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind tcp ip fail");
		return -1;
	}
	
	listen(s_tcpfd, 12);
	
#ifdef SELECT_SUPPORT
	select_set(s_tcpfd);
#endif
	return 0;
}

int socket_tcp_server_accept(int fd)
{
	int rw;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	rw = accept(fd, (struct sockaddr *)&client_addr, &len);
	
#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *m_list;
	
	m_list = (tcp_conn_t *)malloc(sizeof(tcp_conn_t));
	m_list->fd = rw;
	m_list->tclient = COMM_TCLIENT;
	m_list->client_addr = client_addr;
	m_list->next = NULL;

	if (addto_tcpconn_list(m_list) < 0)
	{
		free(m_list);
		close(rw);
		return 0;
	}
#endif

#ifdef SELECT_SUPPORT
	select_set(rw);
#endif

	return 0;
}

void socket_tcp_server_send(frhandler_arg_t *arg, char *data, int len)
{
	if(arg == NULL)
	{
		return;
	}

	send(arg->fd, data, len, 0);

#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *tconn = queryfrom_tcpconn_list(arg->fd);
	if(tconn != NULL)
	{
		arg->addr = tconn->client_addr;
	}
#endif
}
#endif

#if defined(TRANS_TCP_SERVER)
void socket_tcp_server_release(int fd)
{
	close(fd);
#ifdef SELECT_SUPPORT
	select_clr(fd);
#endif

#ifdef DE_PRINT_TCP_PORT
#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *m_list = queryfrom_tcpconn_list(fd);
	if(m_list != NULL)
	{
		trans_data_show(DE_TCP_RELEASE, &m_list->client_addr, "", 0);
	}
	delfrom_tcpconn_list(fd);
#else
	AI_PRINTF("TCP:release,fd=%d\n\n", fd);
#endif
#endif
}

int socket_tcp_server_recv(int fd)
{
	int nbytes;
	char buf[MAXSIZE];
	
	memset(buf, 0, sizeof(buf));
   	if ((nbytes = recv(fd, buf, sizeof(buf), 0)) <= 0)
   	{
      	socket_tcp_server_release(fd);
	}
	else
	{
#ifdef TRANS_TCP_CONN_LIST
		tcp_conn_t *tconn = queryfrom_tcpconn_list(fd);
		frhandler_arg_t *frarg = 
			get_frhandler_arg_alloc(fd, &tconn->client_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
		//tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
		//analysis_capps_frame(frarg);
#endif
#endif
	}
	return 0;
}
#endif

#ifdef TRANS_TCP_CLIENT
int get_ctcp_fd()
{
	return c_tcpfd;
}

int socket_tcp_client_connect(int port)
{
	if ((c_tcpfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("client tcp socket fail");
		return -1;
	}

	m_server_addr.sin_family = PF_INET;
	m_server_addr.sin_port = htons(port);
	m_server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

	if(connect(c_tcpfd, (struct sockaddr *)&m_server_addr, sizeof(m_server_addr)) < 0)
	{
		perror("client tcp socket connect server fail");
		return -1;
	}
	AI_PRINTF("new tcp client connection: fd=%d\n", c_tcpfd);

#ifdef SELECT_SUPPORT
	select_set(c_tcpfd);
#endif
	return 0;
}

int socket_tcp_client_recv()
{
	int nbytes;
	char buf[MAXSIZE];
	
	memset(buf, 0, sizeof(buf));
   	if ((nbytes = recv(c_tcpfd, buf, sizeof(buf), 0)) <= 0)
   	{
      	socket_tcp_client_close();
	}
	else
	{
		frhandler_arg_t *frarg = 
			get_frhandler_arg_alloc(c_tcpfd, &m_server_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
		//tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
		analysis_capps_frame(frarg);
#endif
	}

	return 0;
}

void socket_tcp_client_send(char *data, int len)
{
	send(c_tcpfd, data, len, 0);
}

void socket_tcp_client_close()
{
	close(c_tcpfd);
	AI_PRINTF("close tcp client connection: fd=%d\n", c_tcpfd);
}
#endif

#if defined(TRANS_UDP_SERVICE)
int get_udp_fd()
{
	return udpfd;
}

int socket_udp_service_init(int port)
{	
	if((udpfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("udp socket fail");
		return -1;
	}

	m_addr.sin_family = PF_INET;
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.1.1");

	if (bind(udpfd, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind udp ip fail");
		return -1;
	}

#ifdef SELECT_SUPPORT
	select_set(udpfd);
#endif
	return 0;
}


int socket_udp_recvfrom()
{
	int nbytes;
	char buf[MAXSIZE];

	struct sockaddr_in client_addr;
	socklen_t socklen = sizeof(client_addr);

	memset(buf, 0, sizeof(buf));
	
	nbytes = recvfrom(udpfd, buf, sizeof(buf), 0, 
				(struct sockaddr *)&client_addr, &socklen);


#if defined(TRANS_UDP_SERVICE)
	frhandler_arg_t *frarg = 
		get_frhandler_arg_alloc(udpfd, &client_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
	//tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
	//analysis_capps_frame(frarg);
#endif
#endif
	return 0;
}

void socket_udp_sendto_with_ipaddr(char *ipaddr, char *data, int len)
{
	int i,dlen;
	char saddr[16] = {0};
	int iport = 0;
	
	dlen = strlen(ipaddr);
	if(ipaddr!=NULL && dlen>0 && dlen<24)
	{
		for(i=0; i<dlen; i++)
		{
			if(*(ipaddr+i) == ':')
				break;
		}

		if(i!=0 && i!=dlen)
		{
			memcpy(saddr, ipaddr, i);
			iport = atoi(ipaddr+i+1);
		}
	}

	if(saddr[0]==0 || iport==0)
	{
		AI_PRINTF("error ip address, ipaddr=%s\n", ipaddr);
		return;
	}
	
	struct sockaddr_in maddr;

	maddr.sin_family = PF_INET;
	maddr.sin_port = htons(iport);
	maddr.sin_addr.s_addr = inet_addr(saddr);

	socket_udp_sendto(&maddr, data, len);
}

void socket_udp_sendto(struct sockaddr_in *addr, char *data, int len)
{
#ifdef TRANS_UDP_SERVICE
	sendto(udpfd, data, len, 0, (struct sockaddr *)addr, sizeof(struct sockaddr));
#endif
}
#endif

#ifdef __cplusplus
}
#endif
