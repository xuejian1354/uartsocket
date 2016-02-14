/*
 * protocol.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <services/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum UTOCOL
{
	UT_TCP,
	UT_UDP,
}utocol_t;

typedef enum UMODE
{
	UM_MAIN,
	UM_SLAVE,
}umode_t;

typedef struct TRSESS
{
	char sn[32];
	char dev[64];
	utocol_t tocol;
	umode_t mode;
	uint32 ip;
	int port;
	int isactive;
	int timeout;
	int refd;
	void *parent;
	void *arg;
	struct TRSESS *next;
}trsess_t;

utocol_t get_utocol_fromstr(char* s);
char *get_utocol_tostr(utocol_t tocol);
char get_utocol_tochr(utocol_t tocol);
umode_t get_umode_fromchr(char c);
char get_umode_tochr(umode_t mode);

void set_session_sn(trsess_t *session);

trsess_t *get_global_session();
int add_global_session(trsess_t *session);
trsess_t *query_global_session(char *sn);
int del_global_session(char *sn);
int add_trans_session(trsess_t **g_session, trsess_t *session);
trsess_t *query_trans_session(trsess_t *g_session, char *sn);
int del_trans_session(trsess_t **g_session, char *sn);
void session_free(trsess_t *g_session);

int transcomm_thread_create(trsess_t *session);

void trsess_print(trsess_t *session);

#ifdef __cplusplus
}
#endif

#endif //__PROTOCOL_H__