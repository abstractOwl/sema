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
// sems[0] - table 1 mutex
// sems[1] - table 2 mutex
// sems[2] - ugrad[0]'s sema
// sems[3] - ugrad[1]'s sema
// sems[4] - grad sema
int sems[6];

void sig_int_h(int num)
{
	printf("Student %d ate %d times.\n", student_num, count);
	exit(EXIT_SUCCESS);
}

// Invoke a student with id `num` who does `what`.
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

void do_grad(int num)
{
	printf("[GRAD ] Howdy.\n");
	while (1) {
		// Do grad student things

		// Try to acquire seat
		sem_down(sems[4]);
		sem_down(sems[num % 2]);
		printf("[GRAD ] Grad[%d] eating\n", num);
        count++;
		//sleep(1);

		// Increment the semaphore of U-Grad
		sem_up(sems[2]);
		sem_up(sems[3]);

		// Walk away from table
		sem_up(sems[num % 2]);
	}
}

void do_ugrad(int num)
{
	printf("[UGRAD] Yo!\n");
	while (1) {
		int i;
		for (i = 0; i < GRAD_NUM; i++) {
			sem_down(sems[num % 2 + 2]);
		}

		// Synchronize with other U-Grad
		sem_up(sems[3 - (num % 2)]);
		sem_down(sems[num % 2 + 2]);

		// Eat at table
		sem_down(sems[num % 2]);
		printf("[UGRAD] U-Grad[%d] eating\n", num);
        count++;
        //sleep(1);

		// Synchronize again with other U-Grad
		sem_up(sems[3 - (num % 2)]);
		sem_down(sems[num % 2 + 2]);

		for (i = 0; i < GRAD_NUM / 2; i++) {
			sem_up(sems[4]);
		}

		// If there's a remainder
		if ((GRAD_NUM % 2 & num % 2) == 1) {
			sem_up(sems[4]);
		}
		sem_up(sems[num % 2]);
	}
}

void clean_up()
{
	int i;
	for (i = 0; i < STUDENT_NUM; i++) {
		kill(pids[i], SIGINT);
	}
}

// main loop
int main(void)
{
	int i;

	sems[0] = sem_init(1);
	sems[1] = sem_init(1);
	sems[2] = sem_init(0);
	sems[3] = sem_init(0);
	sems[4] = sem_init(GRAD_NUM);

	for (i = 0; i < STUDENT_NUM; i++) {
		pids[i] = invoke((i < GRAD_NUM) ? &do_grad : &do_ugrad, i);
	}

	sleep(10);
	clean_up();

	return 0;
}

