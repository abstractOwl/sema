#include "sema.h"

#define INITIAL_SIZE 10
#define EXPAND_RATIO 1.5

#ifndef OK
#define OK 0
#endif

/**
 * This file contains the main program of a semaphore service implementation
 * for MINIX.
 */

// Structs
typedef struct
{
	endpoint_t *value;
	link_t     *next;
} link_t;

typedef struct
{
	unsigned int value;
	endpoint_t   *next;
} semaphore_t;


// Define vars

semaphore_t	*semaphores;
size_t			sem_len;				// Tracks the current size of the semaphore array
size_t			tail_pos;				// Tracks the position of the slot after the last
														//   initialized semaphore
size_t			min_empty_pos;	// Tracks the minimum open slot


// Function signatures
void die(const char *message);
void log(const char *message);
int  next_empty_pos();

int do_sem_up				(message *msg);
int do_sem_down			(message *msg);
int do_sem_release	(message *msg);
int do_sem_init			(message *msg);


// Utility functions

/**
 * Prints an error message to stderr and exits.
 */
void die(const char *message)
{
	fprintf(stderr, "sema.c [ERROR]: %s\n", message);
	exit(EXIT_FAILURE);
}

/**
 * Logs a message to stdout.
 */
void log(const char *message)
{
	fprintf(stdout, "sema.c [INFO ]: %s\n", message);
}

int next_empty_pos()
{
	// There can be no empty slots before min_empty_pos since min_empty_pos is
	// updated each sem_release
	int i;
	for (i = min_empty_pos + 1; i < sem_len; i++) {
		if (semaphores[i] == NULL) {
			return i;
		}
	}
}


// Semaphore Functions
/**
 * Initializes the semaphore array.
 */
int init_sem()
{
	sem_len				= INITIAL_SIZE;
	semaphores		= (semaphore_t *) malloc(sizeof (semaphore_t) * sem_len);
	tail_pos			= 0;
	min_empty_pos	= 0;
}

int do_sem_up(message *msg)
{
	log("SEM_UP received.");

	// Increment value

	// If there are items on the queue, process them

	return OK;
}
int do_sem_down(message *msg)
{
	log("SEM_DOWN received.");

	// If value == 0, add this item to queue

	// Else, decrement value

	return OK;
}
int do_sem_release(message *msg)
{
	log("SEM_RELEASE received.");

	return OK;
}
int do_sem_init(message *msg)
{
	log("SEM_INIT received.");
	return OK;
}


struct machine machine;

// sema.c main function
int main(void)
{
	endpoint_t	who;			// Caller endpoint
	message			msg;			// Incoming message
	int					call_nr;	// System call number
	int					result;		// Result to system call
	int					rv;
	int					s;

	// SEF local startup
	sef_startup();
	if ((s = sys_getmachine(&machine)) != OK) {
		die("Could not get machine info");
	}

	// Initialize service
	init_sem();

	// Main loop
	while (TRUE) {
		int ipc_status;

		// Wait for next message
		if (sef_receive_status(ANY, &msg, &ipc_status)) {
			die("sef_receive_status error");
		}

		who				= msg.m_source;	// Sender endpoint
		call_nr		= msg.m_type;		// System call number

		if (is_ipc_notify(ipc_status)) {
			switch (who) {
				case CLOCK:
					expire_timers(msg.NOTIFY_TIMESTAMP);
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
				result = no_sys(who_e, call_nr);
			}
		}

		if (result != SUSPEND) {
			msg.m_type = result;
			if (send(who, &msg) != OK) {
				log("Unable to send reply to endpoint");
			}
		}
	}

	return OK;
}
