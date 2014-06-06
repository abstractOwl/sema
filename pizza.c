#include <sys/types.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sema.h>
#include <unistd.h>

#define GRAD_NUM    6
#define UGRAD_NUM   2
#define STUDENT_NUM (GRAD_NUM + UGRAD_NUM)

unsigned int student_num;
unsigned int count = 0;

pid_t pids[8];
/**
 * sems[0] - table 1 mutex
 * sems[1] - table 2 mutex
 * sems[2] - ugrad[0]'s sema
 * sems[3] - ugrad[1]'s sema
 * sems[4] - grad sema
 */
int sems[6];


// Catch SIGINT, print stats, and exit
void sig_int_h(int num)
{
	printf("Student %d ate %d times.\n", student_num, count);
	exit(EXIT_SUCCESS);
}

/**
 * Creates a student child thread.
 *
 * @param what Function pointer to the student's main loop
 * @param num  Numerical ID to identify the student
 */
pid_t invoke(void (*what)(int), int num)
{
	pid_t pid = fork();
	if (pid == -1) {
		// Fork error :(
		perror("ERROR");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		student_num = num;
		signal(SIGINT, sig_int_h);
		what(num);
		_exit(EXIT_SUCCESS);
	}
	return pid;
}

/**
 * Do grad student things.
 *
 * @param num Numerical ID to identify the grad student
 */
void do_grad(int num)
{
	printf("[GRAD ] Howdy.\n");
	while (1) {
		// Try to acquire seat
		sem_down(sems[4]);
		sem_down(sems[num % 2]);
		printf("[GRAD ] Grad[%d] eating at table %d\n", num, num % 2 + 1);
		count++;
		//sleep(1);

		// Increment the semaphore of U-Grad
		sem_up(sems[2]);
		sem_up(sems[3]);

		// Walk away from table
		sem_up(sems[num % 2]);
	}
}

/**
 * Do ugrad student things.
 *
 * @param num Numerical ID to identify the undergrad student
 */
void do_ugrad(int num)
{
	printf("[UGRAD] Yo!\n");
	while (1) {
		int i;
		for (i = 0; i < GRAD_NUM; i++) {
			sem_down(sems[num % 2 + 2]);
		}

		// Eat at table
		sem_down(sems[num % 2]);
		printf("[UGRAD] U-Grad[%d] eating at table %d\n", num, num % 2 + 1);
		count++;
		//sleep(1);

		for (i = 0; i < GRAD_NUM / 2; i++) {
			sem_up(sems[4]);
		}
		// If there's a remainder
		if ((GRAD_NUM % 2 & num % 2) == 1) {
			sem_up(sems[4]);
		}

		// Synchronize again with other U-Grad
		sem_up(sems[3 - (num % 2)]);
		sem_down(sems[num % 2 + 2]);

		sem_up(sems[num % 2]);
	}
}

/**
 * Sends SIGINT to all child processes.
 */
void clean_up()
{
	int i;
	for (i = 0; i < STUDENT_NUM; i++) {
		kill(pids[i], SIGINT);
	}
}

/**
 * pizza.c main(). Sleeps for 10s before terminating children processes.
 */
int main(void)
{
	int i;

	// Tables each seat one
	sems[0] = sem_init(1);
	sems[1] = sem_init(1);

	// U-Grads initially wait
	sems[2] = sem_init(0);
	sems[3] = sem_init(0);

	// Allocate about one slice for each grad
	sems[4] = sem_init(GRAD_NUM);

	// Create students
	for (i = 0; i < STUDENT_NUM; i++) {
		pids[i] = invoke((i < GRAD_NUM) ? &do_grad : &do_ugrad, i);
	}

	// Wait 10s before exiting
	sleep(10);
	clean_up();

	return 0;
}

