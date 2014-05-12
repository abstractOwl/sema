/* Function prototypes */

#define _POSIX_SOURCE 1
#define _MINIX        1
#define _SYSTEM       1

#include <stdlib.h>
#include <stdio.h>

#include <minix/config.h>
#include <sys/types.h>
#include <minix/const.h>

#include <minix/com.h>
#include <minix/syslib.h>
#include <minix/sysutil.h>

#include <errno.h>

extern struct machine machine;

/* main.c */
int do_sem_up      (message *msg);
int do_sem_down    (message *msg);
int do_sem_release (message *msg);
int do_sem_init    (message *msg);
int main(void);
