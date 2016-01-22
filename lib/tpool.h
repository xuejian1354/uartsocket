/*
 * tpool.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#ifndef __T_POOL_H__
#define __T_POOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

typedef enum
{
	TPOOL_NONE,
	TPOOL_LOCK,
}tpool_opt_t;

typedef void (*routine_t)(void *);

/* excute task list */
typedef struct tpool_work {
    routine_t 	routine;	/* task func */
    void		*arg;		/* task param */
    tpool_opt_t options;
    struct tpool_work	*next;
} tpool_work_t;

typedef struct tpool {
    int			shutdown;	/* is destory */
    int			max_thr_num;	/* max thread num */
    pthread_t		*thr_id;	/* thread id array */
    tpool_work_t	*queue_head;	/* thread task list */
    pthread_mutex_t	queue_lock;
    pthread_mutex_t	func_lock;
    pthread_cond_t	queue_ready;
} tpool_t;

int tpool_create(int max_thr_num);
void tpool_destroy();
int tpool_add_work(routine_t routine, void *arg, tpool_opt_t options);
#endif //__TPOOL_H__
