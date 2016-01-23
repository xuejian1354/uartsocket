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

#define AI_PRINTF(format, args...)	   \
st(    \
	printf(format, ##args);    \
)

#ifdef __cplusplus
}
#endif

#endif  //__DLOG_H__
