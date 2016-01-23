/*
 * mconfig.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#include <mtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_POOL_SUPPORT
#define TIMER_SUPPORT
#define SELECT_SUPPORT
//#define TRANS_TCP_SERVER
//#define TRANS_TCP_CLIENT
//#define TRANS_UDP_SERVICE
//#define DAEMON_PROCESS_CREATE
//#define SERIAL_SUPPORT
//#define READ_CONF_FILE


#ifdef READ_CONF_FILE
#define CONF_FILE	"/etc/uartsocket.conf"
#endif

#ifdef __cplusplus
}
#endif

#endif //__MCONFIG_H__

