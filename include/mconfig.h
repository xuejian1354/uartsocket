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
#define UART_COMMBY_SOCKET
//#define TRANS_TCP_SERVER
//#define TRANS_TCP_CLIENT
//#define TRANS_UDP_SERVICE
//#define DAEMON_PROCESS_CREATE
//#define SERIAL_SUPPORT
//#define READ_CONF_FILE
//#define DE_TRANS_UDP_STREAM_LOG
//#define DE_TRANS_UDP_CONTROL


#if defined(DB_API_WITH_MYSQL) && defined(DB_API_WITH_SQLITE)
#error "cannot define DB_API_WITH_MYSQL with DB_API_WITH_SQLITE at same time"
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DAEMON_PROCESS_CREATE
#endif

#define BIND_SUPERBUTTON_CTRL_SUPPORT

#if defined(TRANS_TCP_SERVER) && defined(TRANS_TCP_CLIENT)
#error 'cannot define TRANS_TCP_SERVER and TRANS_TCP_CLIENT at the same time'
#endif

#ifdef LOAD_BALANCE_SUPPORT
#define BALANCE_SERVER_FILE		"/etc/balance_serlist"
#endif

#ifdef READ_CONF_FILE
#define CONF_FILE	"/etc/uartsocket.conf"
#endif

#if !defined(TRANS_UDP_SERVICE) && defined(REMOTE_UPDATE_APK)
#error 'You must define TRANS_UDP_SERVICE first before defining REMOTE_UPDATE_APK'
#endif

#ifdef __cplusplus
}
#endif

#endif //__MCONFIG_H__

