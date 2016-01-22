/* globals.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <mconfig.h>
#include <debug/dconfig.h>
#include <debug/dlog.h>
#include <signal.h>
#include <tpool.h>
#include <strings_t.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mincode.h>
#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

//default program name
#ifndef TARGET_NAME
#define TARGET_NAME "uartsocket(default)"
#endif

#define SERVER_IP	"loongsmart.com"

#define SERVER_GW_LIST_MAX_NUM	1024
#define SERVER_CLI_LIST_MAX_NUM		128
#define GATEWAY_CLI_LIST_MAX_NUM	24

#define ZDEVICE_MAX_NUM		256

//max connection size of tcp
#define TRANS_TCP_CONN_MAX_SIZE		1024

//max udp session size of udp
#define TRANS_UDP_SESS_MAX_SIZE		1024

#if defined(TRANS_TCP_SERVER) || defined(UART_COMMBY_SOCKET)
#define TRANS_TCP_CONN_LIST
#endif

//serial dev
#define TRANS_SERIAL_DEV	"/dev/ttyS1"
//replace uart server socket port
#define UART_REPORT		11431

//udp protocol using port
#define TRANS_UDP_PORT	11432
#define TRANS_UDP_REMOTE_PORT	TRANS_UDP_PORT

//tcp client protocol using port
#define TRANS_TCPCLIENT_PORT	11433

//tcp server protocol using port
#define TRANS_TCPSERVER_PORT	11434

#if defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
#define DEU_UDP_CMD			"deudp"
#define DEU_TCP_CMD			"detcp"
#define DEU_POST_CMD		"depost"
#define DEU_UART_CMD		"deuart"
#define DE_UDP_PORT			13688
#define DE_UDP_CTRL_PORT	13689
#endif

#define GLOBAL_CONF_COMM_PROTOCOL	"comm_protocol"
#define GLOBAL_CONF_SERIAL_PORT		"serial_dev"
#define GLOBAL_CONF_MAIN_IP			"main_ip"
#define GLOBAL_CONF_TCP_PORT		"tcp_port"
#define GLOBAL_CONF_UDP_PORT		"udp_port"

#define GLOBAL_CONF_ISSETVAL_PROTOCOL		0x00000001
#define GLOBAL_CONF_ISSETVAL_SERIAL			0x00000002
#define GLOBAL_CONF_ISSETVAL_IP				0x00000004
#define GLOBAL_CONF_ISSETVAL_TCP			0x00000008
#define GLOBAL_CONF_ISSETVAL_UDP			0x00000020

#define GLOBAL_TRANSTOCOL_SIZE		4


/*
Old version not ed_type on gateway frame
This macro just support that
*/
//transport layer listening connection max number
#define TRANS_SERVER_THREAD_MAX_NUM		12
#define TRANS_CLIENT_THREAD_MAX_NUM		6

//single frame max size
#define MAXSIZE	0x4000

#define IP_ADDR_MAX_SIZE	24

#define GET_UDP_SERVICE_IPADDR(ipaddr)								\
st(															\
	sprintf(ipaddr, "%s:%d", get_server_ip(), get_udp_port());	\
)

#define CMDLINE_SIZE	0x4000
#define GET_CMD_LINE()	cmdline

#define SET_CMD_LINE(format, args...)  	\
st(  									\
	bzero(cmdline, sizeof(cmdline));  	\
	sprintf(cmdline, format, ##args);  	\
)

typedef byte zidentify_no_t[8];
typedef byte cidentify_no_t[8];

typedef enum
{
	TOCOL_NONE = 0,
	TOCOL_UDP = 0x01,
	TOCOL_TCP_CLIENT = 0x02,
	TOCOL_TCP_SERVER = 0x04,
}transtocol_t;

typedef struct
{
	uint32 isset_flag;
	transtocol_t transtocols;
	int tocol_len;

#ifdef SERIAL_SUPPORT
	char serial_dev[16];
#endif

#if defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT)
	char main_ip[IP_ADDR_MAX_SIZE];
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	int tcp_port;
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	int udp_port;
#endif
}global_conf_t;

typedef struct ConfVal
{
	int head_pos;
	char *val;
	struct ConfVal *next;
}confval_list;

#ifdef DE_TRANS_UDP_STREAM_LOG
char *get_de_buf();
#endif

char *get_serial_dev();
void set_serial_dev(char *name);

int get_tcp_port();
int get_udp_port();
void set_tcp_port(int port);
void set_udp_port(int port);

int start_params(int argc, char **argv);
char *get_time_head();
#ifdef DAEMON_PROCESS_CREATE
int daemon_init();
int get_daemon_cmdline();
#endif
int mach_init();
void event_init();

global_conf_t *get_global_conf();
#ifdef READ_CONF_FILE
int conf_read_from_file();
void translate_confval_to_str(char *dst, char *src);
confval_list *get_confval_alloc_from_str(char *str);
void get_confval_free(confval_list *pval);
char *get_val_from_name(char *name);
#endif
char *get_current_time();
char *get_system_time();

#ifdef __cplusplus
}
#endif

#endif	//__GLOBALS_H__
