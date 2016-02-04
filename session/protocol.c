/*
 * protocol.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "protocol.h"

static trsess_t *g_sess = NULL;

utocol_t get_utocol_fromstr(char* s)
{
	
	if(!strcmp(s, "tcp"))
	{
		return UT_TCP;
	}
	else
	{
		return UT_UDP;
	}
}

char *get_utocol_tostr(utocol_t tocol)
{
	switch(tocol)
	{
	case UT_TCP:
		return "tcp";

	default:
		return "udp";
	}
}

char get_utocol_tochr(utocol_t tocol)
{
	switch(tocol)
	{
	case UT_TCP:
		return '1';

	default:
		return '2';
	}
}

umode_t get_umode_fromchr(char c)
{
	switch(c)
	{
	case '1':
		return UM_MAIN;

	default:
		return UM_SLAVE;
	}
}

char get_umode_tochr(umode_t mode)
{
	switch(mode)
	{
	case UM_MAIN:
		return '1';

	default:
		return '2';
	}
}

void set_session_sn(trsess_t *session)
{
	char ipxs[12] = {0};
	char portxs[6] = {0};
	incode_xtoc32(ipxs, session->ip);
	incode_xtoc16(portxs, session->port);

	memset(session->sn, 0, sizeof(session->sn));
	sprintf(session->sn, "%c%c%s%s",
							get_utocol_tochr(session->tocol),
							get_umode_tochr(session->mode),
							ipxs,
							portxs);
}

trsess_t *get_global_session()
{
	return g_sess;
}

int add_trans_session(trsess_t *session)
{
	
}

int del_trans_session(char *sn)
{

}
