/*
 * strings_t.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __STRINGS_T_H__
#define __STRINGS_T_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define STRS_MEMCPY(dst, src, dstlen, srclen)    \
do{                                              \
  if(dst == NULL || src == NULL) break;          \
  int len = dstlen>srclen?srclen:(dstlen-1);     \
  memset(dst, 0, dstlen);						 \
  memcpy(dst, src, len);                         \
}while(0)

typedef struct
{
	char **str;
	unsigned int size;
}strings_t;

strings_t *strings_alloc(unsigned int size);
int strings_add(strings_t *strs, char *str);
void strings_free(strings_t *strs);

#ifdef __cplusplus
}
#endif

#endif	//__STRINGS_T_H__
