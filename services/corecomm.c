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
static sigset_t sigmask;

int select_init()
{
	maxfd = 0;
	FD_ZERO(&global_rdfs);

	
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

void select_clr(int fd)
{
	FD_CLR(fd, &global_rdfs);
}

int select_listen()
{
	int i, ret;
#if defined(TRANS_UDP_SERVICE)
	int udpfd = get_udp_fd();
#endif
	
	fd_set current_rdfs = global_rdfs;
	ret = pselect(maxfd+1, &current_rdfs, NULL, NULL, NULL, &sigmask);
	if(ret > 0)
	{
#if defined(TRANS_UDP_SERVICE)
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
#if defined(TRANS_TCP_SERVER)
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
		AI_PRINTF("%s()%d : pselect error\n", __FUNCTION__, __LINE__);
	}
	else
	{
		AI_PRINTF("%s()%d : None fd select\n", __FUNCTION__, __LINE__);
	}
	
	usleep(10000);

	return 0;
}
#endif

#ifdef __cplusplus
}
#endif
