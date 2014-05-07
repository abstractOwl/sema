#include "sema.h"

/**
 * This file contains the main program of a semaphore service implementation
 * for MINIX.
 */


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

int main(void)
{
		int call_nr;		// System call number
		int who_e;			// Caller endpoint
		int result;			// Result to system call
		int rv;
		int s;

    message  m_in;	// Incoming message

		//
		sef_local_startup();
		if ((s = sys_getmachine(&machine)) != OK) {
			die("Could not get machine info");
		}

		// Initialize service
		//init_sema(); // TODO: Implement this function

		// Main loop
		while (TRUE) {
			int ipc_status;

			if (sef_receive_status(ANY, &m_in, &ipc_status) != OK) {
				die("sef_receive_status error");
			}
			who_e   = m_in.m_source;  // Sender endpoint
			call_nr = m_in.m_type;    // Syscall number

			// Check for system notifications first
			if (is_ipc_notify(ipc_status)) {
				switch (who_e) {
				case CLOCK:
					expire_timers(m_in.NOTIFY_TIMESTAMP);
					continue; // Don't reply
				default:
					result = ENOSYS;
				}

				if (result != SUSPEND) {
					// Send reply
					m_in.m_type = result;
					int s = send(who_e, &m_in);
					if (s != OK) {
						printf("SEMA: Unable to reply to endpoint %d, code %d\n", who_e, s);
					}
				}
			} else {
				switch (call_nr) {
					case SEM_DOWN:
						log("SEM_DOWN received.");
						break;
					case SEM_INIT:
						log("SEM_INIT received.");
						break;
					case SEM_RELEASE:
						log("SEM_RELEASE received.");
						break;
					case SEM_UP:
						log("SEM_UP received.");
						break;
					default:
						result = no_sys(who_e, call_nr);
				}
			}
		}

    return OK;
}
