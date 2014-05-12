#ifndef _MINIX_SEMA_H
#define _MINIX_SEMA_H

int sem_init    (int start_value);
int sem_down    (int semaphore_number);
int sem_up      (int semaphore_number);
int sem_release (int semaphore_number)

#endif /* _MINIX_SEMA_H */
