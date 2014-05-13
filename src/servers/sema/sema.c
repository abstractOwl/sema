#include "sema.h"

#define INITIAL_SIZE  10
#define EXPAND_FACTOR 1.5

#define DEBUG
#ifdef DEBUG
#define DIE(msg, ...) { \
	fprintf(stdout, "sema.c [ERROR]: " msg "\n", __VA_ARGS__); \
	exit(EXIT_FAILURE); \
}
#define LOG(msg, ...) fprintf(stdout, "sema.c [INFO ]: " msg "\n", __VA_ARGS__)
#else
#define DIE(...)
#define LOG(...)
#endif

#ifndef OK
#define OK 0
#endif

/**
 * This file contains the main program of a semaphore service implementation
 * for MINIX.
 */


typedef struct link_t
{
	endpoint_t    value;
	struct link_t *next;
} link_t;

typedef struct semaphore_t
{
	unsigned int value;
	link_t       *head;
	link_t       *tail;
	int          in_use;
} semaphore_t;


semaphore_t	*semaphores;
size_t			sem_len;				// Tracks the current size of the semaphore array
size_t			tail_pos;				// Tracks the position of the slot after the last
														//   initialized semaphore
size_t			min_empty_pos;	// Tracks the minimum open slot


// Utility functions

static int next_empty_pos()
{
	// There can be no empty slots before min_empty_pos since min_empty_pos is
	// updated each sem_release
	int i;
	for (i = min_empty_pos + 1; i < sem_len; i++) {
		if (semaphores[i].in_use == 0) {
			return i;
		}
	}
	return -1;
}


// Semaphore Functions
/**
 * Initializes the semaphore array.
 */
static int init_sem()
{
	int i;

	log("Initializing semaphore service...");

	sem_len				= INITIAL_SIZE;
	semaphores		= (semaphore_t *) malloc(sizeof (semaphore_t) * sem_len);
	tail_pos			= 0;
	min_empty_pos	= 0;

	if (semaphores == 0) {
		LOG("init_sem(): Ran out of memory");
		return ENOMEM;
	}

	// Because malloc does not zero out memory
	for (i = 0; i < sem_len; i++) {
		semaphores[i].in_use = 0;
	}

	LOG("Semaphore service initialized.");
	return OK;
}

int do_sem_up(message *msg)
{
	semaphore_t *sem;
	LOG("SEM_UP received.");

	// Bounds check
	if (msg->SEM_VALUE == 0 ||
			&(semaphores[msg->SEM_VALUE]) > &(semaphores[sem_len])) {
		return EINVAL;
	}

	sem = &semaphores[msg->SEM_VALUE];
	
	// Check that semaphore is initialized
	if (sem->in_use == 0) {
		return EINVAL;
	}

	if (sem->head == NULL) {
		// Increment value
		++sem->value;
	} else {
		// if there are items on the queue, process one
		
		// Notify endpoint
		message msg;
		msg.SEM_VALUE = sem->value;
		send(sem->head->value, &msg);

		// Dequeue
		link_t *tmp = sem->head;
		sem->head = sem->head->next;
		free(tmp);
	}

	return OK;
}
int do_sem_down(message *msg)
{
	semaphore_t *sem;
	LOG("SEM_DOWN received.");

	// Bounds check
	if (msg->SEM_VALUE == 0 ||
			&(semaphores[msg->SEM_VALUE]) > &(semaphores[sem_len])) {
		return EINVAL;
	}

	sem = &semaphores[msg->SEM_VALUE];
	
	// Check that semaphore is initialized
	if (sem->in_use == 0) {
		return EINVAL;
	}

	if (sem->value == 0) {
		// if value == 0, add this item to queue
		link_t *ep = malloc(sizeof(link_t));
		if (ep == 0) {
			LOG("do_sem_down(): Ran out of memory");
			return ENOMEM;
		}
		ep->value  = msg->m_source;
		ep->next   = NULL;

		if (sem->head == 0) {
			sem->head       = ep;
		} else {
			sem->tail->next = ep;
		}
		sem->tail = ep;
		return SUSPEND;
	} else {
		// else, decrement value
		--sem->value;
		return OK;
	}
}
int do_sem_release(message *msg)
{
	LOG("SEM_RELEASE received.");

	if (semaphores[msg->SEM_NUM].head != 0) {
		LOG("do_sem_release(): Tried to release active semaphore");
		return EINUSE;
	}

	semaphores[msg->SEM_NUM].in_use = 0;

	// Update min empty pos ptr
	if (min_empty_pos == -1 || min_empty_pos > msg->SEM_NUM) {
		min_empty_pos = msg->SEM_NUM;
	}

	return OK;
}
int do_sem_init(message *msg)
{
	LOG("SEM_INIT received.");

	// Find empty slot
	int sem_index;
	if (tail_pos < sem_len) {
		sem_index = tail_pos;
		tail_pos++;
	} else if (min_empty_pos != -1) {
		sem_index = min_empty_pos;
		min_empty_pos = next_empty_pos();
	} else {
		semaphore_t* tmp =
			(semaphore_t *) realloc(semaphores, sem_len * EXPAND_FACTOR);
		if (tmp == 0) {
			LOG("do_sem_init(): Ran out of memory");
			return ENOMEM;
		}
		semaphores = tmp;
		sem_index  = tail_pos;
		tail_pos++;
	}

	// Initialize new semaphore at sem_index
	semaphores[sem_index].value  = msg->SEM_VALUE;
	semaphores[sem_index].head   = NULL;
	semaphores[sem_index].tail   = NULL;
	semaphores[sem_index].in_use = 1;

	return sem_index;
}


struct machine machine;

// sema.c main function
int main(void)
{
	endpoint_t	who;			// Caller endpoint
	message			msg;			// Incoming message
	int					call_nr;	// System call number
	int					result;		// Result to system call
	int					s;

	// SEF local startup
	sef_startup();
	if ((s = sys_getmachine(&machine)) != OK) {
		DIE("Could not get machine info: %d", s);
	}

	// Initialize service
	init_sem();

	// Main loop
	while (TRUE) {
		int rv;
		int ipc_status;

		// Wait for next message
		if (rv = sef_receive_status(ANY, &msg, &ipc_status)) {
			DIE("sef_receive_status error: %d", rv);
		}

		who				= msg.m_source;	// Sender endpoint
		call_nr		= msg.m_type;		// System call number

		if (is_ipc_notify(ipc_status)) {
			switch (who) {
				case CLOCK:
					// expire_timers(msg.NOTIFY_TIMESTAMP);
					continue;
				default:
					result = ENOSYS;
			}
		} else {
			switch (call_nr) {
			case SEM_DOWN:
				result = do_sem_down(&msg);
				break;
			case SEM_INIT:
				result = do_sem_init(&msg);
				break;
			case SEM_RELEASE:
				result = do_sem_release(&msg);
				break;
			case SEM_UP:
				result = do_sem_up(&msg);
				break;
			default:
				result = EINVAL;
			}
		}

		if (result != SUSPEND) {
			int rv;
			msg.m_type = result;
			if (rv = send(who, &msg) != OK) {
				LOG("Unable to send reply to endpoint %d: %d", who, rv);
			}
		}
	}

	return OK;
}
