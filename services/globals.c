/* globals.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "globals.h"
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <arpa/inet.h>
#include <session/protocol.h>

#ifdef __cplusplus
extern "C" {
#endif

static char current_time[64];
static char conf_file[128] = {0};
static int end_flag = 1;

static void end_handler(int sig);

#ifdef READ_CONF_FILE
void get_read_line(char *line, int len);
#endif

int get_end()
{
	return end_flag;
}

int start_params(int argc, char **argv)
{
	int ch;
	opterr = 0; 
	sprintf(conf_file, "%s", CONF_FILE);

	const char *optstrs = "c:h";

    while((ch = getopt(argc, argv, optstrs)) != -1)
    {
		switch(ch)
		{
		case 'h':
			AI_PRINTF("Usage: %s [-c <Configuration files>]\n", argv[0]);
			AI_PRINTF("Default: \n\t-c %s\n", CONF_FILE);
			return 1;

		case '?':
			AI_PRINTF("Unrecognize arguments.\n");
			AI_PRINTF("\'%s -h\' get more help infomations.\n", argv[0]);
			return 1;

		case 'c':
			memset(conf_file, 0, sizeof(conf_file));
			sprintf(conf_file, "%s", optarg);
			break;
		}
	}

	return 0;
}

#ifdef READ_CONF_FILE
int conf_read_from_file()
{
	FILE *fp = NULL;
	char buf[1024] = {0};

	if((fp = fopen(conf_file, "r")) != NULL)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			get_read_line(buf, strlen(buf));	
			memset(buf, 0, sizeof(buf));
		}

		fclose(fp);
	}
	else
	{
		AI_PRINTF("%s()%d :  Read \"%s\" error, please set configuration file\n", 
			__FUNCTION__, __LINE__, 
			conf_file);
		return -1;
	}

	return 0;
}

void get_read_line(char *line, int len)
{
	int i;
	int is_ignore = 0;
	int pos_h, pos_t;
	int isset_h = 0;

#define FIELDS_LEN	7

	char fields[FIELDS_LEN][128] = {0};
	char flen = 0;

	if(line == NULL)
	{
		return;
	}

	for(i=0; i<len; i++)
	{	
		switch(*(line+i))
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			if(isset_h)
			{
				isset_h = 0;
				if(flen < FIELDS_LEN)
				{
					memcpy(fields[flen++], line+pos_h, i-pos_h);
				}
			}
			break;

		case '#':
			if(!is_ignore)
			{
				return;
			}

		default:
			is_ignore = 1;
			if(!isset_h)
			{
				pos_h = i;
				isset_h = 1;
			}
			break;
		}
	}

	if(flen >= FIELDS_LEN)
	{
		trsess_t *t_session = calloc(1, sizeof(trsess_t));
		strcpy(t_session->dev, fields[0]);
		t_session->tocol = get_utocol_fromstr(fields[1]);
		t_session->mode= get_umode_fromchr(fields[2][0]);
		t_session->ip = inet_addr(fields[3]);
		t_session->port = atoi(fields[4]);
		t_session->isactive = atoi(fields[5]);
		t_session->timeout = atoi(fields[6]);
		t_session->parent = NULL;
		t_session->refd = -1;
		t_session->arg = NULL;

		set_session_sn(t_session);
		if(add_global_session(t_session) != 0)
		{
			free(t_session);
		}
	}
}
#endif

void process_signal_register()
{
	 signal(SIGINT, end_handler);
	 signal(SIGTSTP, end_handler);
}

void end_handler(int sig)
{
	switch(sig)
	{
	case SIGINT:
		AI_PRINTF(" SIGINT\t");
		break;

	case SIGTSTP:
		AI_PRINTF(" SIGTSTP\t");
		break;
	}

	AI_PRINTF("\n");

	session_free(get_global_session());

	end_flag = 0;
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
	
    int ret = chdir("/tmp");
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
	trsess_t *g_session = get_global_session();
	trsess_t *t_session = g_session;

	while(t_session != NULL)
	{
		transcomm_thread_create(t_session);
		t_session = t_session->next;
	}

	return 0;
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

#ifdef __cplusplus
}
#endif

