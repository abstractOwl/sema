/**
 * Header file for the semaphore service.
 */

#ifndef SEMA_H
#define SEMA_H

#include <stdlib.h>
#include <stdio.h>

#include <minix/config.h>
#include <sys/types.h>
#include <minix/const.h>

#include <minix/syslib.h>
#include <minix/sysutil.h>
#include <minix/timers.h>

#include <errno.h>

// Structs
typedef struct link_t
{
	endpoint_t    *value;
	struct link_t *next;
} link_t;

typedef struct semaphore_t
{
	unsigned int value;
	link_t       *next;
	int          in_use;
} semaphore_t;

//#include "proto.h"

//int do_sem_init();
//int do_sem_down();

#endif
