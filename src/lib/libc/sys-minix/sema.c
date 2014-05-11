#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>

#include <minix/rs.h>

static int get_sema_endpt(endpoint_t *pt)
{
	return minix_rs_lookup("sema", pt);
}

int sem_init(int start_value)
{
	endpoint_t sema_ep;
	message msg;

	if (get_sema_endpt(&sema_ep) != OK) {
		return ENOSYS;
	}

	msg.SEM_VALUE = start_value;
	return _syscall(sema_ep, SEM_INIT, &msg);
}

int sem_down(int semaphore_number)
{
	endpoint_t sema_ep;
	message msg;

	if (get_sema_endpt(&sema_ep) != OK) {
		return ENOSYS;
	}

	msg.SEM_NUM = semaphore_number;
	return _syscall(sema_ep, SEM_DOWN, &msg);
}

int sem_up(int semaphore_number)
{
	endpoint_t sema_ep;
	message msg;

	if (get_sema_endpt(&sema_ep) != OK) {
		return ENOSYS;
	}

	msg.SEM_NUM = semaphore_number;
	return _syscall(sema_ep, SEM_UP, &msg);
}

int sem_release(int semaphore_number)
{
	endpoint_t sema_ep;
	message msg;

	if (get_sema_endpt(&sema_ep) != OK) {
		return ENOSYS;
	}

	msg.SEM_NUM = semaphore_number;
	return _syscall(sema_ep, SEM_RELEASE, &msg);
}
