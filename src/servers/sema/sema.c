#include "sema.h"

#define INITIAL_SIZE 10
#define EXPAND_RATIO 0.5

/**
 * This file contains the main program of a semaphore service implementation
 * for MINIX.
 */

int			*semaphores;
size_t	sem_len;				// Tracks the current size of the semaphore array
size_t	tail_pos;				// Tracks the position of the slot after the last
												//   initialized semaphore
size_t	min_empty_pos;	// Tracks the minimum open slot

// Function signatures
void die(const char *message);
void log(const char *message);

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


// Semaphore Functions
/**
 * Initializes the semaphore array.
 */
int init_sem()
{
	sem_len				= INITIAL_SIZE;
	semaphores		= (int *) malloc(sizeof (int) * sem_len);
	tail_pos			= 0;
	min_empty_pos	= 0;
}

int do_sem_up(message *msg)
{
	log("SEM_UP received.");
	return OK;
}
int do_sem_down(message *msg)
{
	log("SEM_DOWN received.");
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


// sema.c main function
int main(void)
{
	int			result;	// Result to system call
	message	msg;		// Incoming message

	// Initialize service
	init_sem();

	// Main loop
	while (TRUE) {
		int ipc_status;

		ipc_receive(&msg);

		switch (msg.m_type) {
		case SEM_DOWN:
			result = do_sem_down(&msg);
			break;
		case SEMA_INIT:
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
		
		if (result != EDONTREPLY) {
			msg.m_type = result;
			ipc_reply(msg.m_source, &msg);
		}
	}

	return OK;
}
