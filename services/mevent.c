/*
 * mevent.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "mevent.h"
#include <module/serial.h>
#include <services/balancer.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TIMER_SUPPORT
static int heartbeat_interval = 500;

static void heartbeat_request(void *p);

void set_heartbeat_check(int immediate, int interval)
{
	timer_event_param_t timer_param;
	heartbeat_interval = interval;
	timer_param.resident = 1;
	timer_param.interval = heartbeat_interval;
	timer_param.count = 1;
	timer_param.immediate = immediate;

	set_mevent(HEARTBEAT_EVENT, heartbeat_request, &timer_param);
}

void heartbeat_request(void *p)
{
}

void set_mevent(int id, timer_callback_t event_callback, timer_event_param_t *param)
{
	timer_event_t *timer_event = (timer_event_t *)calloc(1, sizeof(timer_event_t));
	timer_event->timer_id = id;
	timer_event->param = *param;
	timer_event->timer_callback = event_callback;
	
	if(set_timer_event(timer_event) != 0)
	{
		free(timer_event);
	}
}
#endif

#ifdef __cplusplus
}
#endif
