/*
 * balancer.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "balancer.h"

#ifdef __cplusplus
extern "C" {
#endif

static relser_list_t *p_serlist;
static const char *none_name = "none";
static const char *none_server_ip = "0.0.0.0";

int add_relser_info(relser_info_t *m_ser);
relser_info_t *query_relser_info(char *name);
int del_relser_info(char *name);
line_data_t *serlist_linehandle(char *buf, int len);


int add_relser_info(relser_info_t *m_ser)
{
	relser_info_t *pre_ser =  NULL;
	relser_info_t *t_ser = p_serlist->p_ser;

	if(m_ser == NULL)
	{
		return -1;
	}
	else
	{
		m_ser->next = NULL;
	}

	while(t_ser != NULL)
	{
		if(memcmp(t_ser->name, m_ser->name, strlen(t_ser->name)))
		{
			pre_ser = t_ser;
			t_ser = t_ser->next;
		}
		else
		{
			memset(t_ser->ipaddr, 0, sizeof(t_ser->ipaddr));
			memcpy(t_ser->ipaddr, m_ser->ipaddr, strlen((char *)m_ser->ipaddr));

			/*if(pre_ser != NULL)
			{
				pre_ser->next = t_ser->next;
				t_ser->next = p_serlist->p_ser;
				p_serlist->p_ser = t_ser;
			}*/
			
			return 1;
		}
	}

	//m_ser->next = p_serlist->p_ser;
	//p_serlist->p_ser = m_ser;

	if(pre_ser != NULL)
	{
		pre_ser->next = m_ser;
	}
	else
	{
		p_serlist->p_ser = m_ser;
	}
	p_serlist->max_num++;

	return 0;
}

relser_info_t *query_relser_info(char *name)
{
	relser_info_t *t_ser = p_serlist->p_ser;


	while(t_ser != NULL)
	{
		if(memcmp(t_ser->name, name, strlen(t_ser->name)))
		{
			t_ser = t_ser->next;
		}
		else
		{
			return t_ser;
		}
	}

	return NULL;
}

int del_relser_info(char *name)
{
	relser_info_t *pre_ser =  NULL;
	relser_info_t *t_ser = p_serlist->p_ser;


	while(t_ser != NULL)
	{
		if(memcmp(t_ser->name, name, strlen(t_ser->name)))
		{
			pre_ser = t_ser;
			t_ser = t_ser->next;
		}
		else
		{
			if(pre_ser != NULL)
			{
				pre_ser->next = t_ser->next;
			}
			else
			{
				p_serlist->p_ser = t_ser->next;
			}

			free(t_ser);
			p_serlist->max_num--;
			return 0;
		}
	}

	return -1;
}


int serlist_read_from_confile(void)
{
	FILE *fp = NULL;
	char buf[64] = {0};

	p_serlist = (relser_list_t *)calloc(1, sizeof(relser_list_t));
	p_serlist->p_ser = NULL;
	p_serlist->max_num = 0;
		
	if((fp = fopen(BALANCE_SERVER_FILE, "r")) != NULL)
	{
		relser_info_t *relser_info;
		
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			line_data_t *ldata = serlist_linehandle(buf, strlen(buf));
			if(ldata != NULL)
			{
				switch(ldata->type)
				{
				case LINE_NAME:
					relser_info = (relser_info_t *)calloc(1, sizeof(relser_info_t));
					memcpy(relser_info->name, ldata->data, strlen(ldata->data));
					
					if (add_relser_info(relser_info) != 0)
					{
						free(relser_info);
						relser_info = query_relser_info(ldata->data);
					}
					break;
					
				case LINE_IP:
					if(relser_info != NULL)
					{
						memset(relser_info->ipaddr, 0, sizeof(relser_info->ipaddr));
						memcpy(relser_info->ipaddr, ldata->data, strlen(ldata->data));
					}
					break;
					
				case LINE_NONE:
					break;
				}

				free(ldata);
			}
			
			memset(buf, 0, sizeof(buf));
		}

		if(p_serlist->max_num == 0)
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

line_data_t *serlist_linehandle(char *buf, int len)
{
	switch(buf[0])
	{
	case '[':
	{
		int i, isFind = 0;

		for(i=0; i<len; i++)
		{
			if(buf[i] == ']')
			{
				isFind = 1;
				break;
			}
		}

		if(isFind && i>1)
		{
			line_data_t *line_data = (line_data_t *)calloc(1, sizeof(line_data_t));
			line_data->type = LINE_NAME;
			memcpy(line_data->data, buf+1, i-1);
			return line_data;
		}
	}
	break;

	case 'i':
	{
		if(memcmp(buf, "ip=", 3) || len <= 3)
		{
			break;
		}
		
		line_data_t *line_data = (line_data_t *)calloc(1, sizeof(line_data_t));
		line_data->type = LINE_IP;
		memcpy(line_data->data, buf+3, len-4);
		return line_data;
	}
	break;

	default: break;
	}

	return NULL;
}

char *get_server_ip(void)
{
	global_conf_t *g_conf = get_global_conf();
#if defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT)
	if(g_conf->isset_flag & GLOBAL_CONF_ISSETVAL_IP)
	{
		return g_conf->main_ip;
	}
#endif

	if(p_serlist != NULL)
	{
		return (char *)p_serlist->p_ser->ipaddr;
	}

	return (char *)none_server_ip;
}

char *get_server_ip_from_name(char *name)
{
	relser_info_t *t_ser = NULL;
	if(p_serlist != NULL)
	{
		t_ser = p_serlist->p_ser;
	}
	
	while(t_ser != NULL)
	{
		if(!memcmp(t_ser->name, name, strlen(t_ser->name)))
		{
			return (char *)t_ser->ipaddr;
		}
		t_ser = t_ser->next;
	}

	return (char *)none_server_ip;
}

char *get_server_name_from_ip(char *ip)
{
	relser_info_t *t_ser = NULL;
	if(p_serlist != NULL)
	{
		t_ser = p_serlist->p_ser;
	}
	
	while(t_ser != NULL)
	{
		if(!memcmp(t_ser->ipaddr, ip, strlen((char *)t_ser->ipaddr)))
		{
			return t_ser->name;
		}
	}

	return (char *)none_name;
}

#ifdef __cplusplus
}
#endif
