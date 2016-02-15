/*
 * protocol.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "protocol.h"
#include <module/netlist.h>

static trsess_t *g_sess = NULL;

void incode_xtocs(char *dest , unsigned char *src, int len);
void incode_xtoc16(char *dest, unsigned short src);
void incode_xtoc32(char *dest, unsigned int src);

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

int add_global_session(trsess_t *session)
{
	return add_trans_session(&g_sess, session);
}

trsess_t *query_global_session(char *sn)
{
	return query_trans_session(g_sess, sn);
}

int del_global_session(char *sn)
{
	return del_trans_session(&g_sess, sn);
}

int add_trans_session(trsess_t **g_session, trsess_t *session)
{
	trsess_t *pre_sess = NULL;
	trsess_t *t_sess = *g_session;


	if(session == NULL)
	{
		return -1;
	}
	else
	{
		session->next = NULL;
	}

	while(t_sess != NULL)
	{
		if(strcmp(t_sess->sn, session->sn))
		{
			pre_sess = t_sess;
			t_sess = t_sess->next;
		}
		else
		{
			strcpy(t_sess->dev, session->dev);
			t_sess->speed = session->speed;
			t_sess->tocol = session->tocol;
			t_sess->mode = session->mode;
			t_sess->ip = session->ip;
			t_sess->port = session->port;
			t_sess->timeout = session->timeout;
			t_sess->parent = session->parent;

			if(pre_sess != NULL)
			{
				pre_sess->next = t_sess->next;
				t_sess->next = *g_session;
				*g_session = t_sess;
			}

			return 1;
		}
	}

	session->next = *g_session;
	*g_session = session;

	return 0;
}

trsess_t *query_trans_session(trsess_t *g_session, char *sn)
{
	trsess_t *t_sess = g_session;

	while(t_sess != NULL)
	{
		if(strcmp(t_sess->sn, sn))
		{
			t_sess = t_sess->next;
		}
		else
		{
			return t_sess;
		}
	}

	return NULL;
}

int del_trans_session(trsess_t **g_session, char *sn)
{
	trsess_t *pre_sess = NULL;
	trsess_t *t_sess = *g_session;

	while(t_sess != NULL)
	{
		if(strcmp(t_sess->sn, sn))
		{
			pre_sess = t_sess;
			t_sess = t_sess->next;
		}
		else
		{
			if(pre_sess != NULL)
			{
				pre_sess->next = t_sess->next;
			}
			else
			{
				*g_session = t_sess->next;
			}

			free(t_sess);
			return 0;
		}
	}

	return -1;
}

void session_free(trsess_t *g_session)
{
	trsess_t *m_session = g_session;
	while(m_session != NULL)
	{
		//trsess_print(m_session);
		trsess_t *t_session = m_session;
		m_session = m_session->next;

		if(t_session->tocol == UT_TCP && t_session->mode == UM_MAIN)
		{
			tcp_conn_t *t_conn = (tcp_conn_t *)t_session->arg;
			while(t_conn != NULL)
			{
				tcp_conn_t *pre_conn = t_conn;
				t_conn = t_conn->next;
				free(pre_conn);
			}
		}

		free(t_session);
	}
}

int transcomm_thread_create(trsess_t *session)
{
	if(session == NULL)
	{
		return -1;
	}

	serial_init(session);
}

void trsess_print(trsess_t *session)
{
	if(session == NULL)
	{
		AI_PRINTF("(NULL)\n");
		return;
	}

	AI_PRINTF("session %s\n\tdev:\t%s\n\ttocol:\t%s\n\tmode:\t%c\n\tip:\t%04X\n\tport:\t%d\n\tisactive:\t%d\n\ttimeout:\t%d\n\n",
				session->sn,
				session->dev,
				get_utocol_tostr(session->tocol),
				get_umode_tochr(session->mode),
				session->ip,
				session->port,
				session->isactive,
				session->timeout);
}

void incode_xtocs(char *dest , unsigned char *src, int len)
{
    int i, temp;
	if(len<1 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<len; i++)
	{
		temp = (*(src+i)>>4);
		if(temp < 0xA)
		{
			dest[(i<<1)] = temp + '0';	
		}
		else
		{
			dest[(i<<1)] = temp - 0xA + 'A';	
		}
		
		temp = (*(src+i)&0x0F);
		if(temp < 0xA)
		{
			dest[(i<<1)+1] = temp + '0';	
		}
		else
		{
			dest[(i<<1)+1] = temp - 0xA + 'A';	
		}
	}
}


void incode_xtoc16(char *dest, unsigned short src)
{
	unsigned char val[2];
	val[0] = (src>>8);
	val[1] = (src&0xFF);
	incode_xtocs(dest, val, 2);
}

void incode_xtoc32(char *dest, unsigned int src)
{
	unsigned char val[4];
	val[0] = ((src>>24)&0xFF);
	val[1] = ((src>>16)&0xFF);
	val[2] = ((src>>8)&0xFF);
	val[3] = (src&0xFF);
	
	incode_xtocs(dest, val, 4);
}