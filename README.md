
# CS170 P2: Pizza Synchronization
AbstractOwl
Jon Adler <jadler@umail>


## Intro
   For our pizza sharing implementation, we implemented a FIFO-like scheduling
algorithm to ensure no starvation. Because the "eating" action is expected to
last approximately the same time for each student, we are able to use FIFO,
expecting a maximum delay of EAT_TIME each round (not including semaphore
delays).


## Algorithm
   This implementation essentially uses a producer-consumer model to replicate
FIFO behavior. To do this, we used the 5 semaphores described in [Figure 1]
below:


  +-------------+------------------------------------------------------------+
  | sems index  | description                                                |
  +=============+============================================================+
  | 0           | Mutex for table 1, initialized to 1                        |
  +-------------+------------------------------------------------------------+
  | 1           | Mutex for table 2, initialized to 1                        |
  +-------------+------------------------------------------------------------+
  | 2           | Semaphore for undergrad student 1, initialized to 0        |
  +-------------+------------------------------------------------------------+
  | 3           | Semaphore for undergrad student 2, initialized to 0        |
  +-------------+------------------------------------------------------------+
  | 4           | Semaphore for graduate students, initialized to 6          |
  +-------------+------------------------------------------------------------+
  Figure 1: Semaphores in `sems` array


   At the beginning, the table mutexes are initialized to 1, allowing for one
student at each table. Each student is assigned to a table based on his/her
student number. The grad students get to  eat first.

   First, the graduate student decrements the graduate student semaphore by 1
and waits to be seated at a table. Every time a grad student eats, he/she
increments both ugrad students' semaphores by 1 before releasing the table
mutex. Finally, the grad student goes back to waiting for the grad semaphore
to be incremented.

   Meanwhile, the ugrad students decrement their respective semaphores once for
each grad student (6 times). Now it is the ugrads' turn for pizza. Once they
finish eating, they restore the grad student semaphore to 6. They synchronize
by incrementing each others' semaphore and decrementing their own to ensure
both are finished eating. Finally, they release their tables and go back to
waiting.

    While there are opportunities for graduate students to steal each others'
food, the semaphore waiting queue ensures that those who miss out get pizza
first the next round.


## Results
   In our tests, we found that the students were allocated pizza quite fairly.
In a couple unscientific test runs, the all students hovered around 1800-1900
slices of pizza in 10 seconds.
