/* globals.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "globals.h"
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <module/netapi.h>
#include <services/mevent.h>

#ifdef __cplusplus
extern "C" {
#endif

static char current_time[64];
static char port_buf[16];

static char serial_dev[16] = TRANS_SERIAL_DEV;
static int tcp_port = TRANS_TCPCLIENT_PORT;
static int udp_port = TRANS_UDP_REMOTE_PORT;

static global_conf_t g_conf = 
{
	0,
	TOCOL_NONE,
	3,
#ifdef SERIAL_SUPPORT
	TRANS_SERIAL_DEV,
#endif
#if defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT)
	{0},
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	TRANS_TCPCLIENT_PORT,
#endif
#if defined(TRANS_UDP_SERVICE)
	TRANS_UDP_PORT,
#endif
};

#ifdef READ_CONF_FILE
static void get_read_line(char *line, int len);
static void set_conf_val(char *cmd, char *val);
static int get_conf_setval();
#endif

static char cur_time[64];

char *get_serial_dev()
{
	return serial_dev;
}

void set_serial_dev(char *name)
{
	memset(serial_dev, 0, sizeof(serial_dev));
	strcpy(serial_dev, name);
}

int get_tcp_port()
{
	return tcp_port;
}

int get_udp_port()
{
	return udp_port;
}

void set_tcp_port(int port)
{
	tcp_port = port;
}

void set_udp_port(int port)
{
	udp_port = port;
}

int start_params(int argc, char **argv)
{

	int ch;
	opterr = 0;  
	global_conf_t t_conf = {0};


#ifdef SERIAL_SUPPORT
  #if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
    #if defined(TRANS_UDP_SERVICE)
	const char *optstrs = "s:t:u:h";
	#else
	const char *optstrs = "s:t:h";
    #endif
  #elif defined(TRANS_UDP_SERVICE)
	const char *optstrs = "s:u:h";
  #else
  	const char *optstrs = "s:h";
  #endif
#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  #if defined(TRANS_UDP_SERVICE)
  	const char *optstrs = "t:u:h";
  #else
  	const char *optstrs = "t:h";
  #endif
#elif defined(TRANS_UDP_SERVICE)
	const char *optstrs = "u:h";
#else
	const char *optstrs = "h";
#endif
	
    while((ch = getopt(argc, argv, optstrs)) != -1)
    {
		switch(ch)
		{
		case 'h':
#ifdef SERIAL_SUPPORT
  #if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
    #if defined(TRANS_UDP_SERVICE)
			AI_PRINTF("Usage: %s [-s<Serial Device>] [-t<TCP Port>] [-u<UDP Port>]\n", argv[0]);
			AI_PRINTF("Default: \n\t-s %s\n\t-t %d\n\t-u %d\n",
							TRANS_SERIAL_DEV,
							TRANS_TCPCLIENT_PORT,
							TRANS_UDP_PORT);
	#else
			AI_PRINTF("Usage: %s [-s<Serial Device>] [-t<TCP Port>]\n", argv[0]);
			AI_PRINTF("Default:\n\t-s %s\n\t-t %d\n", TRANS_SERIAL_DEV, TRANS_TCPCLIENT_PORT);
    #endif
  #elif defined(TRANS_UDP_SERVICE)
			AI_PRINTF("Usage: %s [-s<Serial Device>] [-u<UDP Port>]\n", argv[0]);
  			AI_PRINTF("Default:\n\t-s %s\n\t-u %d\n", TRANS_SERIAL_DEV, TRANS_UDP_PORT);
  #else
  			AI_PRINTF("Usage: %s [-s<Serial Device>]\n", argv[0]);
  			AI_PRINTF("Default:\n\t-s %s\n", TRANS_SERIAL_DEV);
  #endif
#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  #if defined(TRANS_UDP_SERVICE)
  			AI_PRINTF("Usage: %s [-t<TCP Port>] [-u<UDP Port>]\n", argv[0]);
  			AI_PRINTF("Default:\n\t-t %d\n\t-u %d\n", TRANS_TCPCLIENT_PORT, TRANS_UDP_PORT);
  #else
  			AI_PRINTF("Usage: %s [-t<TCP Port>]\n", argv[0]);
  			AI_PRINTF("Default:\n\t-t %d\n", TRANS_TCPCLIENT_PORT);
  #endif
#elif defined(TRANS_UDP_SERVICE)
			AI_PRINTF("Usage: %s [-u<UDP Port>]\n", argv[0]);
			AI_PRINTF("Default:\n\t-u %d\n", TRANS_UDP_PORT);
#else
			AI_PRINTF("Usage: %s\n", argv[0]);
#endif
			return 1;

		case '?':
			AI_PRINTF("Unrecognize arguments.\n");
			AI_PRINTF("\'%s -h\' get more help infomations.\n", argv[0]);
			return 1;

#ifdef SERIAL_SUPPORT
		case 's':
			sprintf(t_conf.serial_dev, "%s", optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
			break;
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
		case 't':
			t_conf.tcp_port = atoi(optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			break;
#endif
#if defined(TRANS_UDP_SERVICE)
		case 'u':
			t_conf.udp_port = atoi(optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
			break;
#endif
		}
	}
	
#ifdef SERIAL_SUPPORT
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_SERIAL)
	{
		set_serial_dev(t_conf.serial_dev);
	}
	else
	{
  #ifdef READ_CONF_FILE
		set_serial_dev(g_conf.serial_dev);
  #else
		set_serial_dev(TRANS_SERIAL_DEV);
  #endif
	}
	
  #if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_TCP)
  	{
		set_tcp_port(t_conf.tcp_port);
  	}
	else
	{
	#ifdef READ_CONF_FILE
		set_tcp_port(g_conf.tcp_port);
    #else
		set_tcp_port(TRANS_TCPCLIENT_PORT);
    #endif
	}
	
    #if defined(TRANS_UDP_SERVICE)

	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
	{
		set_udp_port(t_conf.udp_port);
	}
	else
	{
      #ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
      #else
		set_udp_port(TRANS_UDP_PORT);
      #endif
	}
    #endif
	
  #elif defined(TRANS_UDP_SERVICE)
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
  	{
		set_udp_port(t_conf.udp_port);
  	}
	else
	{
	#ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
    #else
		set_udp_port(TRANS_UDP_PORT);
    #endif
	}
  #endif

#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_TCP)
  	{
		set_tcp_port(t_conf.tcp_port);
  	}
	else
	{
  #ifdef READ_CONF_FILE
		set_tcp_port(g_conf.tcp_port);
  #else
		set_tcp_port(TRANS_TCPCLIENT_PORT);
  #endif
	}

  #if defined(TRANS_UDP_SERVICE)
  	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
  	{
		set_udp_port(t_conf.udp_port);
  	}
	else
	{
    #ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
    #else
		set_udp_port(TRANS_UDP_PORT);
    #endif
	}
  #endif
  
#elif defined(TRANS_UDP_SERVICE)
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
  	{
		set_udp_port(t_conf.udp_port);
  	}
	else
	{
    #ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
    #else
		set_udp_port(TRANS_UDP_PORT);
    #endif
	}
#else
#warning "No Comm protocol be selected, please set uart, tcp or udp."
#endif

	AI_PRINTF("%s Start!\n", TARGET_NAME);

#ifdef SERIAL_SUPPORT
	AI_PRINTF("Serial device: \"%s\"\n", get_serial_dev());
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	AI_PRINTF("TCP transmit port: %d\n", get_tcp_port());
#endif

#if defined(TRANS_UDP_SERVICE)
	AI_PRINTF("UDP transmit port: %d\n", get_udp_port());
#endif

	return 0;
}


char *get_time_head()
{
	time_t t;
	bzero(cur_time, sizeof(cur_time));
	time(&t);
	struct tm *tp= localtime(&t);
	strftime(cur_time, 100, "[%Y-%m-%d-%H:%M:%S] ", tp);

	return cur_time;
}

#ifdef DAEMON_PROCESS_CREATE
static int daemon_cmdline_flag = 0;

int daemon_init()
{
    int pid;
	
    if(pid = fork())
	{
		return 1;
    }
	else if(pid < 0)
	{
		AI_PRINTF("%s()%d : excute failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

    setsid();
	
	if(pid = fork())
	{
		return 1;
	}
    else if(pid < 0)
	{
		AI_PRINTF("%s()%d : excute failed\n", __FUNCTION__, __LINE__);
		return -1;
    }

	int i;
	int fdtablesize = getdtablesize();
    for(i = 0; i < fdtablesize; i++)
    {
        close(i);
    }
	
    chdir("/tmp");
    umask(0);

	daemon_cmdline_flag = 1;
    return 0;
}

int get_daemon_cmdline()
{
	return daemon_cmdline_flag;
}
#endif

int mach_init()
{
	return 0;
}

void event_init()
{
#ifdef TIMER_SUPPORT
	set_heartbeat_check(0, 500);
#endif
}

global_conf_t *get_global_conf()
{
	return &g_conf;
}

#ifdef READ_CONF_FILE
int conf_read_from_file()
{
	FILE *fp = NULL;
	char buf[128] = {0};
	g_conf.isset_flag = 0;
	g_conf.transtocols = TOCOL_NONE;
	g_conf.tocol_len = 0;

	if((fp = fopen(CONF_FILE, "r")) != NULL)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			get_read_line(buf, strlen(buf));	
			memset(buf, 0, sizeof(buf));
		}
	}
	else
	{
		AI_PRINTF("%s()%d :  Read \"%s\" error, please set configuration file\n", 
			__FUNCTION__, __LINE__, 
			CONF_FILE);
		return -1;
	}

	if(get_conf_setval() < 0)
	{
		return -1;
	}

	return 0;
}

void get_read_line(char *line, int len)
{
	int i, s;
	int is_ignore = 0;

	int cmd_ph, cmd_pt, val_ph, val_pt;
	char p_isset = 0x07;
	
	if(line == NULL || len < 3)
	{
		return;
	}
	
	for(i=0; i<len; i++)
	{	
		switch(*(line+i))
		{
		case ' ':
			break;

		case '#':
			if(!is_ignore)
			{
				return;
			}
			
		default:
			is_ignore = 1;
			if(p_isset & 0x01)
			{
				cmd_ph = i;
				p_isset &= ~0x01;
			}
			else if(p_isset & 0x02)
			{
				if(*(line+i) == '=')
				{
					int j;
					for(j=i-1; j>=cmd_ph; j--)
					{
						if(*(line+j) != ' ')
						{
							cmd_pt = j+1;
							p_isset &= ~0x02;
							break;
						}
					}
				}
			}
			else if(p_isset & 0x04)
			{
				val_ph = i;
				p_isset &= ~0x04;
			}
			
			break;
		}
	}

	for(s=len-2; s>=val_ph; s--)
	{
		if(*(line+s) != ' ')
		{
			val_pt = s+1;
			break;
		}
	}

	if((cmd_ph < cmd_pt)
		&& (cmd_pt < val_ph)
		&& (val_ph < val_pt))
	{
		char cmd[64] = {0};
		char val[64] = {0};

		memcpy(cmd, line+cmd_ph, cmd_pt-cmd_ph);
		memcpy(val, line+val_ph, val_pt-val_ph);

		set_conf_val(cmd, val);
	}
}

void set_conf_val(char *cmd, char *val)
{
	if(!strcmp(cmd, GLOBAL_CONF_COMM_PROTOCOL))
	{
		int i, len=strlen(val);
		int start_pos, end_pos;
		int start_isset = 0;
		int end_isset = 0;
		int pro_index = 0;
		uint16 transtocol_hasset = TOCOL_NONE;

		for(i=0; i<=len; i++)
		{
			if(start_isset && (end_isset || i == len))
			{
				int field_len = end_pos - start_pos;
				if(i == len && !end_isset)
				{
					field_len++;
				}

				start_isset = 0;
				end_isset = 0;

				if(!(transtocol_hasset & TOCOL_UDP) && field_len == 3
					&& !strncmp(val+start_pos, "udp", field_len))
				{
					pro_index++;
					transtocol_hasset |= TOCOL_UDP;
				}
				else if(!(transtocol_hasset & TOCOL_TCP_CLIENT) && field_len == 9
					&& !strncmp(val+start_pos, "tcpclient", field_len))
				{
					pro_index++;
					transtocol_hasset |= TOCOL_TCP_CLIENT;
				}
				else if(!(transtocol_hasset & TOCOL_TCP_SERVER) && field_len == 9
					&& !strncmp(val+start_pos, "tcpserver", field_len))
				{
					pro_index++;
					transtocol_hasset |= TOCOL_TCP_SERVER;
				}

				if(i == len)
				{
					break;
				}
			}

			if(!start_isset)
			{
				if(*(val+i) == ' '
					|| *(val+i) == ',')
				{
					continue;
				}
				else
				{
					start_pos = i;
					end_pos = i+1;
					start_isset = 1;
				}
			}
			else if(!end_isset)
			{
				end_pos = i;
				if(*(val+i) == ',')
				{
					int j = end_pos;
					while(j > start_pos+1)
					{
						if(*(val+j-1) == ' ')
						{
							j--;
						}
						else
						{
							break;
						}
					}
					end_pos = j;
					end_isset = 1;
				}
			}
		}

		if(pro_index)
		{
			g_conf.transtocols = transtocol_hasset;
			g_conf.tocol_len = pro_index;
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_PROTOCOL;
		}
	}

#ifdef SERIAL_SUPPORT
	if(!strcmp(cmd, GLOBAL_CONF_SERIAL_PORT))
	{
		strcpy(g_conf.serial_dev, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
	}
#endif
#if defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT)
	if(!strcmp(cmd, GLOBAL_CONF_MAIN_IP))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			strcpy(g_conf.main_ip, get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
			get_confval_free(pval);
		}
		else
		{
			strcpy(g_conf.main_ip, val);
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
		}
	}
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	if(!strcmp(cmd, GLOBAL_CONF_TCP_PORT))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			g_conf.tcp_port = atoi(get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			get_confval_free(pval);
		}
		else
		{
			g_conf.tcp_port = atoi(val);
			if(g_conf.tcp_port > 0)
			{
				g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			}
		}
	}
#endif

#if defined(TRANS_UDP_SERVICE)
	if(!strcmp(cmd, GLOBAL_CONF_UDP_PORT))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			g_conf.udp_port = atoi(get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
			get_confval_free(pval);
		}
		else
		{
			g_conf.udp_port = atoi(val);
			if(g_conf.udp_port > 0)
			{
				g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
			}
		}
	}
#endif
}

int get_conf_setval()
{
	int i;
	uint32 issetflags[] = {
					GLOBAL_CONF_ISSETVAL_PROTOCOL,
#ifdef SERIAL_SUPPORT
					GLOBAL_CONF_ISSETVAL_SERIAL,
#endif
#if defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_ISSETVAL_IP,
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_ISSETVAL_TCP,
#endif
#if defined(TRANS_UDP_SERVICE)
					GLOBAL_CONF_ISSETVAL_UDP,
#endif
	};

	const char *issetvals[] = {
					GLOBAL_CONF_COMM_PROTOCOL,
#ifdef SERIAL_SUPPORT
					GLOBAL_CONF_SERIAL_PORT,
#endif
#if defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_MAIN_IP,
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_TCP_PORT,
#endif
#if defined(TRANS_UDP_SERVICE)
					GLOBAL_CONF_UDP_PORT,
#endif
	};


	for(i=0; i<sizeof(issetflags)/sizeof(uint32); i++)
	{
		if(!(g_conf.isset_flag & issetflags[i]))
		{
			AI_PRINTF("%s()%d : val \"%s\" is not set in \"%s\"\n",
							__FUNCTION__, __LINE__, 
							issetvals[i],
							CONF_FILE);
			
			return -1;
		}
	}
	
	return 0;
}

void translate_confval_to_str(char *dst, char *src)
{
	int i;
	int head_isset = 0;
	int head_pos = 0;
	int tail_pos = 0;
	int vallen = strlen(src);

	confval_list *pval = get_confval_alloc_from_str(src);

	for(i=0; i<vallen; i++)
	{
		if(!head_isset && *(src+i) == '{')
		{
			head_isset = 1;
			head_pos = i;
		}
		else if(head_isset && i > head_pos+2 && *(src+i) == '}')
		{
			char str_field[64] = {0};
			memcpy(str_field, src+tail_pos, head_pos-tail_pos);
			strcat(dst, str_field);
			
			confval_list *t_confval = pval;
			while(t_confval != NULL)
			{
				if(t_confval->head_pos == head_pos)
				{
					strcat(dst, get_val_from_name(t_confval->val));
				}
				t_confval = t_confval->next;
			}

			tail_pos = i + 1;
			head_isset = 0;
		}
	}

	if(tail_pos < vallen)
	{
		char str_field[64] = {0};
		memcpy(str_field, src+tail_pos, vallen-tail_pos);
		strcat(dst, str_field);
	}

	get_confval_free(pval);
}

confval_list *get_confval_alloc_from_str(char *str)
{
	int i;
	int vallen = strlen(str);
	int head_isset = 0;
	int head_pos;
	confval_list *pval = NULL;
	
	for(i=0; i<vallen; i++)
	{
		if(!head_isset && *(str+i) == '{')
		{
			head_isset = 1;
			head_pos = i;
		}
		else if(head_isset && i > head_pos+2 && *(str+i) == '}')
		{
			int j;
			int valhead_isset = 0;
			int valhead_pos;

			for(j=head_pos+1; j<=i; j++)
			{
				if(!valhead_isset && *(str+j) == '$')
				{
					valhead_pos = j;
					valhead_isset = 1;
				}
				else if(valhead_isset
						&& (j == i
							|| *(str+j) == ' '
							|| *(str+j) == ','
							|| *(str+j) == ';'
							|| *(str+j) == '$'))
				{
					if(j-valhead_pos > 1)
					{
						int valname_len = j - valhead_pos - 1;
						char *valname = (char *)calloc(1, valname_len+1);
						memcpy(valname, str+valhead_pos+1, valname_len);
						confval_list *m_confval = (confval_list *)calloc(1, sizeof(confval_list));
						m_confval->head_pos = head_pos;
						m_confval->val = valname;
						m_confval->next = NULL;

						if(pval == NULL)
						{
							pval = m_confval;
						}
						else
						{
							confval_list *t_confval = pval;
							while(t_confval->next != NULL)
							{
								t_confval = t_confval->next;
							}
							
							t_confval->next = m_confval;
						}
					}

					if(*(str+j) == '$')
					{
						valhead_pos = j;
					}
					else
					{
						valhead_isset = 0;
					}
				}
			}

			head_isset = 0;
		}
	}

	return pval;
}


void get_confval_free(confval_list *pval)
{
	confval_list *t_confval = pval;
	while(t_confval != NULL)
	{
		confval_list *pre_confval = t_confval;
		t_confval = t_confval->next;
		
		free(pre_confval->val);
		free(pre_confval);
	}
}

char *get_val_from_name(char *name)
{
	if(!strcmp(name, "server_ip"))
	{
		return SERVER_IP;
	}
	else if(!strcmp(name, "default_tcp_port"))
	{
		bzero(port_buf, sizeof(port_buf));
		sprintf(port_buf, "%d", TRANS_TCPCLIENT_PORT);
		return port_buf;
	}
	else if(!strcmp(name, "default_udp_port"))
	{
		bzero(port_buf, sizeof(port_buf));
		sprintf(port_buf, "%d", TRANS_UDP_PORT);
		return port_buf;
	}

	return (char *)"";
}

char *get_current_time()
{
	time_t t;
	time(&t);
	bzero(current_time, sizeof(current_time));
	struct tm *tp= localtime(&t);
	strftime(current_time, 100, "%Y-%m-%d %H:%M:%S", tp); 

	return current_time;
}

char *get_system_time()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	bzero(current_time, sizeof(current_time));
	sprintf(current_time, "%u", (uint32)time.tv_sec);

	return current_time;
}
#endif

#ifdef __cplusplus
}
#endif

