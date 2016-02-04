/*
 * corecomm.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __CORECOMM_H__
#define __CORECOMM_H__

#include <services/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

int select_init();
void select_set(int fd);
void select_clr(int fd);
int select_listen();

#ifdef __cplusplus
}
#endif

#endif  //__CORECOMM_H__