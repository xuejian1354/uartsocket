/*
 * main.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include <services/globals.h>
#include <services/etimer.h>
#include <services/corecomm.h>
#include <module/serial.h>
#include <module/netapi.h>

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char **argv)
{
#ifdef READ_CONF_FILE
	if(conf_read_from_file() < 0)
	{
		return -1;
	}
#endif
	
	if(start_params(argc, argv) != 0)
	{
		return 1;
	}

#ifdef DAEMON_PROCESS_CREATE
	signal(SIGCHLD, SIG_IGN);
    if(daemon_init() != 0)
    {
		return 0;
	}
#endif

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_CLIENT_THREAD_MAX_NUM) < 0)
	{
		return -1;
	}
#endif

#ifdef TIMER_SUPPORT
	if(timer_init() < 0)
	{
		return -1;
	}
#endif

#ifdef SELECT_SUPPORT
	if(select_init() < 0)
	{
		return -1;
	}
#endif

#ifdef SERIAL_SUPPORT
	if(serial_init(get_serial_dev()) < 0)			//init serial device
	{
		return -1;
	}
#endif

#if defined(TRANS_UDP_SERVICE)
	if (socket_udp_service_init(get_udp_port()) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_TCP_CLIENT
	if (socket_tcp_client_connect(get_tcp_port()) < 0)
	{
		return -1;
	}
#endif

	if(mach_init() < 0)
	{
		return -1;
	}

	event_init();

	while(1)
	{
#ifdef SELECT_SUPPORT
		select_listen();
#else
		usleep(10000);
#endif
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
