#include <stdio.h> 
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void semaphore_routine(sem_t *sema, int semaphore_id, char *resource_name); 
			
int main() {
	// mapping semaphore to share between processes
	sem_t *sema = mmap(NULL, 3 * sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			
	// check for mapping error 
	if (sema == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	
	// init 3 semaphores to be used with processes
	// semaphores represent pseudo shared resources: 5 printers, 6 plotters, and 4 scanners
	if (sem_init(sema, 1, 5) != 0 || sem_init(sema + 1, 1, 6) != 0 || sem_init(sema + 2, 1, 4) != 0) {
		perror("sem_init");
		exit(EXIT_FAILURE);	
	}
	
	int proc_amount = 100; // set the amount of child processes to fork() in routine

	// routine creates n child processes to access semphores
	for (int i = 0; i < proc_amount; i++) {
		int rand_num = rand() % 3; // random # 0 - 2 to determine what sempahore to use
		if (fork() == 0) { // child process
			// pick 1 of 3 semaphores at random 
			switch (rand_num) { 
				case 0:
					semaphore_routine(sema, 0, "printers");
					break;
				case 1:
					semaphore_routine(sema, 1, "plotters");
					break;
				case 2:
					semaphore_routine(sema, 2, "scanners");
					break;
			}
			exit(0); // child process exits
		}
	}

	// parent waits for child processes 
	for (int i = 0; i < proc_amount; i++) {
		wait(NULL);
	}

	// destory semaphores
	if (sem_destroy(sema) != 0 || sem_destroy(sema + 1) != 0 || sem_destroy(sema + 2) != 0) {
		perror("error destroying semaphore");
	}

	// unmap semaphores
	if (munmap(sema, 3 * sizeof(sem_t)) != 0) {
		perror("error unmapping");
	}

	return 0;
}

// access semaphore 
void semaphore_routine(sem_t *sema, int semaphore_num, char *resource_name) {
	int sem_val;
	sem_getvalue(sema + semaphore_num, &sem_val);
	// Check if semaphore is unavailable
	if (sem_val == 0) {
		printf("\nNo %s %s", resource_name, "available. Waiting...\n");
		sleep(rand() % 4 + 2); // sleep 2 - 5 seconds
	}	
	sem_wait(sema + semaphore_num);
	sem_getvalue(sema + semaphore_num, &sem_val);
	printf("\nUsing %s %s %d %s", resource_name, "[", sem_val, "left]\n");
	sleep(rand() % 5 + 1); // sleep 1 - 5 seconds
	printf("\nLeaving %s %s", resource_name, "\n");
	sem_post(sema + semaphore_num);
}