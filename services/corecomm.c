/*
 * corecomm.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "corecomm.h"
#include <signal.h>
#include <module/netapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SELECT_SUPPORT

static int maxfd;
static fd_set global_rdfs;
static fd_set global_wtfs;
static sigset_t sigmask;

int select_init()
{
	maxfd = 0;
	FD_ZERO(&global_rdfs);
	FD_ZERO(&global_wtfs);

	
	if(sigemptyset(&sigmask) < 0)
    {
		perror("sigemptyset");
		return -1;
    }

    if(sigaddset(&sigmask,SIGALRM) < 0)
    {
		perror("sigaddset");
		return -1;
    }

	return 0;
}

void select_set(int fd)
{
	FD_SET(fd, &global_rdfs);
	maxfd = maxfd>fd?maxfd:fd;
}

void select_wtset(int fd)
{
	FD_SET(fd, &global_wtfs);
	maxfd = maxfd>fd?maxfd:fd;
}

void select_clr(int fd)
{
	FD_CLR(fd, &global_rdfs);
}

void select_wtclr(int fd)
{
	FD_CLR(fd, &global_wtfs);
}

int select_listen()
{
	int i, ret;
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	int udpfd = get_udp_fd();
#endif
	
	fd_set current_rdfs = global_rdfs;
	fd_set current_wtfs = global_wtfs;
	ret = pselect(maxfd+1, &current_rdfs, &current_wtfs, NULL, NULL, &sigmask);
	if(ret > 0)
	{
#ifdef UART_COMMBY_SOCKET
		int reser_fd = get_reser_fd();
		if(reser_fd >= 0 && FD_ISSET(reser_fd, &current_rdfs))
		{
			return get_reser_accept(reser_fd);
		}
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
		if(FD_ISSET(udpfd, &current_rdfs))
		{
			return socket_udp_recvfrom();
		}
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
		else 
#endif
#endif

#ifdef TRANS_TCP_CLIENT
		if(FD_ISSET(get_ctcp_fd(), &current_rdfs))
		{
			return socket_tcp_client_recv(get_ctcp_fd());
		}
#endif

#ifdef TRANS_TCP_SERVER
		if(FD_ISSET(get_stcp_fd(), &current_rdfs))
		{
			return socket_tcp_server_accept(get_stcp_fd());
		}
#endif
#if defined(TRANS_TCP_SERVER) || defined(UART_COMMBY_SOCKET)
		for(i=0; i<=maxfd; i++)
		{
			if(FD_ISSET(i, &current_rdfs))
			{
				return socket_tcp_server_recv(i);
			}
		}
#endif
	}
	else if (ret < 0)
	{
		DE_PRINTF(0, "%s()%d : pselect error\n", __FUNCTION__, __LINE__);
	}
	else
	{
		DE_PRINTF(0, "%s()%d : None fd select\n", __FUNCTION__, __LINE__);
	}
	
	usleep(10000);

	return 0;
}
#endif

#ifdef __cplusplus
}
#endif
