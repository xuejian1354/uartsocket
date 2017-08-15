/*
 * datahandler.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#ifndef __DATAHANDLER_H__
#define __DATAHANDLER_H__

#include <mconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TRBUF
{
  unsigned long len;
  unsigned char *data;
} trbuf_t;

typedef struct DHANDLER
{
  char name[64];
  trbuf_t *(*recvcall)(trbuf_t *buf);
  trbuf_t *(*sendcall)(trbuf_t *buf);
} dhandler_t;

dhandler_t *get_datahandler(char *name);


#ifdef __cplusplus
}
#endif

#endif //__DATAHANDLER_H__

