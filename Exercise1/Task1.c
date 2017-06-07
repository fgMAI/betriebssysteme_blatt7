#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define THREADCOUNT 10

/* Declarations, Structs, whatever */

void xerror(const char *msg);
static void *thread_main(void *arg);
static void thread_cleanup(void *arg);

typedef struct thread_info
{
	int thread_num;		//this is the sequential number given by main()
	
	//I know that this might not be a reasonable decision, but
	//I didn't know how to 'access' the File-Stream each Thread creates from the
	//thread-cancel handler
	//So, I simply placed the file-pointer in this thread_info struct - which
	//is passed to the cancel-handler ;)  There might be better solutions,
	//but hey - it works :^}
	FILE *fp;
} thread_info;

//wee need those, because threads share the same file-descriptor, address-space, etc.
static pthread_t threadId[THREADCOUNT];
static pthread_mutex_t threadMutex[THREADCOUNT];
static thread_info *threadInfo;


/* Main-Methods */

int main(void)
{
	fprintf(stdout, "[MAIN] Starting...\n");
	//we start by creating a directory for all the threadN.txt files
	struct stat st = {0}; //required for stat
	if(stat("/tmp/Task1", &st) == -1) //creates directory if it doesn't exist
	{
		fprintf(stdout, "[MAIN] Create \"/tmp/Task1\" Directory...\n");
		if(mkdir("/tmp/Task1", 0777) != 0)
			xerror("Could not create directory \"/tmp/Task1\".");
	}

	//randomizeeeer
	srand(time(NULL));

	//create the threadInfo-Fields
	fprintf(stdout, "[MAIN] Allocate ThreadInfo...\n");
	threadInfo = (thread_info*)malloc(sizeof(thread_info) * THREADCOUNT);

	//next we spawn the threads and randomly cancel some
	for(int i = 0; i < THREADCOUNT; i++)
	{
		//initialize the thread_info
		threadInfo[i].thread_num = i;

		//then we create the thread, NULL for default attributes
		fprintf(stdout, "[MAIN] Create Thread #%d...\n", i);
		if(pthread_create(&threadId[i], NULL, thread_main, (threadInfo + i))) //if==0
			xerror("Could not create Thread #"+i);	//pointer-arithmetic FTW

		//next initialize the mutex for this thread
		fprintf(stdout, "[MAIN] Initialize Mutex for Thread #%d...\n", i);
		pthread_mutex_init(&threadMutex[i], NULL);

		//then we check if the thread is lucky enough to stay alive
		int ret = 0;
		if(rand() % 2) //==0
		{
			fprintf(stdout, "[MAIN] Cancelling Thread #%d... Poor dude\n", i);
			ret = pthread_cancel(threadId[i]);
		}

		//verify everything
		if(ret != 0)
			xerror("Error while trying to cancel the thread #" + i);
	}

	//wait for all the threads
	for(int i = (THREADCOUNT - 1); i >= 0; i--)
	{
		fprintf(stdout, "[MAIN] Wait for Thread #%d to finish...\n", i);
		if(pthread_join(threadId[i], NULL) != 0)
			xerror("Error while waiting for terminated thread #" + i);
	}
	
	//and exit :^}
	free(threadInfo);
	exit(EXIT_SUCCESS);
}


/* Thread functions */
static void *thread_main(void *arg)
{
	//initialize the thread
	fprintf(stdout, "[THREAD] Thread started...\n");
	thread_info *threadInfo = arg;
	int tNum = threadInfo->thread_num;
	fprintf(stdout, "[THREAD] Install Handler and set cancel type/state...\n");
	pthread_cleanup_push(thread_cleanup, threadInfo); //install cancel-handler
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //to cancel immediatly on request
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //to actually enable cancellation
	
	//We wait a random time
	fprintf(stdout, "[THREAD] Sleep for some seconds...\n");
	sleep(rand() % 3);
	
	//then create a file, considering the mutex
	fprintf(stdout, "[THREAD] Lock Mutex for Thread %d...\n", tNum);
	pthread_mutex_lock(&threadMutex[tNum]);
	char fileNameBuf[30];
	sprintf(fileNameBuf, "/tmp/Task1/thread%d.txt", tNum);

	fprintf(stdout, "[THREAD] Write ID into File \"%s\"...\n", fileNameBuf);
	threadInfo->fp = fopen(fileNameBuf, "w+"); //append and create if necessary
	if(threadInfo->fp == NULL)
	{
		char buf[2048];
		sprintf(buf, "Could not open File \"%s\" for thread #%d", fileNameBuf, tNum);
		xerror(buf);
	}
	
	fprintf(stdout, "[THREAD] Write ID into File...\n");
	/* I do realize that this is not what was required by the assigenment, but it seems
	* like my system doesn't implement gettid()... At least did I include the necessary
	* headers according to the manpage, but it still doesn't seem to work. sorry ;) */
//	fprintf(threadInfo->fp, "%d", gettid());
	fprintf(threadInfo->fp, "%d", (rand() % 1048576));
	
	//and finish
	fprintf(stdout, "[THREAD] Unlock Mutex and free ressources for Thread %d...\n", tNum);
	pthread_cleanup_pop(1); //1 also executes the thread_cleanup ;)

	exit(EXIT_SUCCESS);
}

void thread_cleanup(void *arg)
{
	//get thread-number
	thread_info *threadInfo = arg;
	int tNum = threadInfo->thread_num;
	
	//close the file
	fclose(threadInfo->fp);

	//and unlock the mutex
	pthread_mutex_unlock(&threadMutex[tNum]);

	//given the circumstances and the situation, we can even destroy the mutex here
	pthread_mutex_destroy(&threadMutex[tNum]);
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




