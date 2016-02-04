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
	if(start_params(argc, argv) != 0)
	{
		return 1;
	}

#ifdef READ_CONF_FILE
	if(conf_read_from_file() < 0)
	{
		return -1;
	}
#endif

	AI_PRINTF("%s Start!\n", TARGET_NAME);

#ifdef DAEMON_PROCESS_CREATE
	signal(SIGCHLD, SIG_IGN);
    if(daemon_init() != 0)
    {
		return 0;
	}
#endif

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(THREAD_MAX_NUM) < 0)
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
