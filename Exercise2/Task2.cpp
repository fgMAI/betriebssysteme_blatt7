#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <queue>

#define THREADCOUNT 4

/* Declarations, Structs, whatever */

void xerror(const char *msg);
static void *thread_main(void *arg);
static void thread_cleanup(void *arg);

//threads
static pthread_t threadId[THREADCOUNT];

//queue
std::queue<int> queue;

//mutex for queue
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;


/* Notice ahead:
	I was not sure if we have to protect with 1 or multiple mutexes (push/pop/empty())
	I did solve it with just a single mutex.
*/


/* Main-Methods */

int main(void)
{
	fprintf(stdout, "[MAIN] Starting...\n");

	//we spawn the threads
	for(int i = 0; i < THREADCOUNT; i++)
	{
		//we create the thread, NULL for default attributes
		fprintf(stdout, "[MAIN] Create Thread #%d...\n", i);
		if(pthread_create(&threadId[i], NULL, thread_main, NULL)) //if==0
			xerror("Could not create Thread #"+i);
	}

	//let's feed the queue with 1's
	fprintf(stdout, "[MAIN] Place 100000 1 into the queue...\n");
	for(int i = 100000; i > 0; i--)
	{
		pthread_mutex_lock(&queueMutex);
		queue.push(1);
		pthread_mutex_unlock(&queueMutex);
	}
	fprintf(stdout, "[MAIN] Place 4 0 into the queue...\n");
	for(int i = THREADCOUNT; i > 0; i--)
	{
		pthread_mutex_lock(&queueMutex);
		queue.push(0);
		pthread_mutex_unlock(&queueMutex);
	}
	
	//wait for all the threads
	for(int i = 0; i < THREADCOUNT; i++)
	{
		fprintf(stdout, "[MAIN] Wait for Thread #%d to finish...\n", i);
		if(pthread_join(threadId[i], NULL) != 0)
			xerror("Error while waiting for terminated thread #" + i);
		fprintf(stdout, "[MAIN] Finished wait for Thread #%d...\n", i);
	}
	
	//and exit :^}
	fprintf(stdout, "[MAIN] Exiting...\n");
	fflush(stdout); //because sometimes message wasn't printed fully
	exit(EXIT_SUCCESS);
}


/* Thread functions */
static void *thread_main(void *arg)
{
	//initialize the thread
	fprintf(stdout, "[THREAD] Thread started...\n");
	int localSum = 0;
	
	//Then we gonna read the queue until we receive a 0
	int nextVal = 1, front;
	while(nextVal != 0)
	{
		//I am not sure if we would have to mutex the empty()-method
		pthread_mutex_lock(&queueMutex);
		if(queue.empty())
		{
			//this is a workaround for while(queue.empty()), while we ensure
			//the access to queue.empty is thread-safe additionally, if 
			//it is not empty, we have already locked the queue for this thread ;)
			pthread_mutex_unlock(&queueMutex); //otherwise unlock it
			continue;
		}
		
		//if it is not empty, we can go ahead and access the value
		if(queue.front() == 0)
		{
			fprintf(stdout, "[THREAD] Thread received \'0\' in queue...\n");
			nextVal = 0;
			pthread_mutex_unlock(&queueMutex);
			continue; //why not break and remove the nextVal :^)
		}

		//if the value != 0, we add it
		front = (int)queue.front();
		queue.pop();
		pthread_mutex_unlock(&queueMutex);
		localSum += front;
	}
	
	//in case we got here, there was a 0 in the queue, go ahead
	fprintf(stdout, "[THREAD] This threads localSum is %d\n", localSum);
	fflush(stdout); //because sometimes message wasn't printed fully
}


/* Helper-Functions */

void xerror(const char *msg)
{
	char output[2048];
	strcpy(output, msg);
	strcat(output, "\n");

	perror(output);fflush(stderr);
	exit(EXIT_FAILURE);
}




