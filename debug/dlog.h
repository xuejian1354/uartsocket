/*
 * dlog.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __DLOG_H__
#define __DLOG_H__

#include <debug/dconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DLOG_FILE   "/var/log/uartsocket.log"

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DE_PRINTF(lwflag, format, args...)  \
st(  \
	FILE *fp = NULL;    \
	char *buf = get_de_buf();  \
	sprintf(buf, format, ##args);  \
	delog_udp_sendto(buf, strlen(buf));	  \
    if(lwflag && (fp = fopen(DLOG_FILE, "a+")) != NULL)   \
    {   \
		fprintf(fp, get_time_head());    \
        fprintf(fp, format, ##args);  \
        fclose(fp); \
    }   \
)
#else
#define DE_PRINTF(lwflag, format, args...)	   \
st(    \
	FILE *fp = NULL;    \
	printf(format, ##args);    \
	if(lwflag && (fp = fopen(DLOG_FILE, "a+")) != NULL)    \
    {    \
		fprintf(fp, get_time_head());    \
        fprintf(fp, format, ##args);    \
        fclose(fp);    \
    }    \
)
#endif

#define PRINT_HEX(data, len)			\
st(										\
	int x;								\
	for(x=0; x<len; x++)				\
	{									\
		DE_PRINTF(0, "%02X ", data[x]);	\
	}									\
	DE_PRINTF(0, "\n");					\
)

#ifdef __cplusplus
}
#endif

#endif  //__DLOG_H__
