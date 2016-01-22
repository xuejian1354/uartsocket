/*
 * balancer.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __BALANCER_H__
#define __BALANCER_H__

#include <services/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define LOAD_BALANCE_SUPPORT

#define RELSER_NAME_NUM		64

typedef enum
{
	LINE_NAME,
	LINE_IP,
	LINE_NONE
}line_data_type_t;

typedef struct
{
	line_data_type_t type;
	char data[64];
}line_data_t;

typedef struct RelSer_info
{
	char name[RELSER_NAME_NUM];
	uint8 ipaddr[IP_ADDR_MAX_SIZE];
	struct RelSer_info *next;
}relser_info_t;

typedef struct RelSer_List
{
	relser_info_t *p_ser;
	int max_num;
}relser_list_t;

int serlist_read_from_confile(void);
char *get_server_ip(void);
char *get_server_ip_from_name(char *name);
char *get_server_name_from_ip(char *ip);

#ifdef __cplusplus
}
#endif

#endif	//__BALANCER_H__