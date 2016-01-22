/*
 * mevent.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __MEVENT_H__
#define __MEVENT_H__

#include <services/globals.h>
#include <services/etimer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HEARTBEAT_EVENT	0x0001

#ifdef TIMER_SUPPORT
void set_heartbeat_check(int immediate, int interval);

void set_mevent(int id, 
	timer_callback_t event_callback, timer_event_param_t *param);
#endif

#ifdef __cplusplus
}
#endif
 
 #endif  //__MEVENT_H__
