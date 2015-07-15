/*
  User Thread Library
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "thread.h"
//#include "memwatch.h"

#define DEBUG(s)	puts(s)

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

#define	CONTEXT_SIZE	5
#define STACK_SIZE	(4096 * 8)

typedef struct _Thread {
    struct _Thread*	next;
    int			thread_id;
    int			context[CONTEXT_SIZE];
    char		*stack_top;	/* NULL if this is main() thread */
    int			status;
} Thread;

#define RUNNING		0
#define FINISH		1

static Thread* currentThread;
static Thread* threadList;

/* in csw-i386.S */
void _ContextSwitch(int* old_context, int* new_context);
void _MakeThread(int* context, char* stack, void (*func)(int, int),
		 int arg1, int arg2);
int _TestAndSet(int* lock);

static void LinkThread(Thread* t);
static void ThreadStart(int proc, int arg);
static Thread* AllocateThread();
static void FreeThread(Thread* t);

int main(int args, char** argv)
{
	void ThreadMain(int, char **);

	threadList = (Thread *) malloc(sizeof(Thread));
	if (threadList == NULL) {
		fprintf(stderr, "Error: can't alloc memory for threadList\n");
		exit(1);
	}
	currentThread = threadList;
	threadList->stack_top = NULL;
	threadList->status = RUNNING;
	threadList->next = NULL;
	threadList->thread_id = 0;
	ThreadMain(args, argv);
	ThreadExit();
	printf("main thread exiting ...\n");
	free(threadList);	

	return 0;
}

int ThreadCreate(ThreadProc proc, int arg)
{
	static int id = 1;
	Thread *t;

	Thread *child = (Thread *) malloc(sizeof(Thread));
	child->stack_top = (char *) malloc(STACK_SIZE);
	if (child == NULL || child->stack_top == NULL) {
		fprintf(stderr, "Error: can't alloc memory for child->stack_top\n");
		exit(1);
	}
	child->thread_id = id++;
	_MakeThread(child->context, child->stack_top + STACK_SIZE,
				ThreadStart, (int)proc, arg);
	child->status = RUNNING;
	child->next = NULL;
	/* current -> child */

	for (t = threadList; t->next != NULL; t = t->next);
	t->next = child;
	printf("[thread list] : ");
	for (t = threadList; t != NULL; t = t->next) {
		printf("%d -> ", t->thread_id);
	}
	printf("\n");
	
	return child->thread_id;
}

static void LinkThread(Thread* t)
{
    Thread* tmp;

    tmp = threadList;
    threadList = t;
    threadList->next = tmp;
}

/*
  ThreadStart() is a procedure that threads first invoke.
*/
static void ThreadStart(int proc, int arg)
{
    ThreadProc ptr = (ThreadProc)proc;
    (*ptr)(arg);
    ThreadExit();
}

/*
  ThreadYield() switches the context.
*/
void ThreadYield()
{
	Thread *t;
	unsigned short found = 0;
	char *p;

	for (t = threadList; t != NULL; t = t->next) {
		if ((t->thread_id != currentThread->thread_id) && (t->status != FINISH)) {
			found = 1;
			printf("thread id = %d is found\n", t->thread_id);
			break;
		}
	}

	if (found) {
		Thread *cur = currentThread;
		currentThread = t;
		currentThread->stack_top = t->stack_top;
		printf("switching...\n");
		_ContextSwitch(cur->context, t->context);
	} else if ((threadList->next == NULL) && (threadList->status == RUNNING)) {
		Thread *cur = currentThread;
		printf("all threads are finished. back to main...\n");
		/* back to main thread */
		currentThread = threadList;
		printf("switching...\n");
		_ContextSwitch(cur->context, threadList->context);
	}

	return;
}

/*
  ThreadExit() finishes the current thread.
  If this thread is main(), this function waits until all tasks finish,
  then normally returns.
*/
void ThreadExit()
{
	static Thread dummy;
	Thread *cur = currentThread;
	Thread *t;
	Thread *prev_t;

	if (cur->stack_top == NULL) {
		cur->status = FINISH;
	} else {
		for (t = threadList, prev_t = t; t != NULL; t = t->next) {
			if (t == cur) {
				prev_t->next = cur->next;
				break;
			}
			prev_t = t;
		}
		dummy.status = FINISH;
		dummy.stack_top = NULL;
		currentThread = &dummy;
		printf("thread %d is freeed\n", cur->thread_id);
		printf("[thread list] : ");
		for (t = threadList; t != NULL; t = t->next) {
			printf("%d -> ", t->thread_id);
		}
		printf("\n");
		free(cur->stack_top);
		free(cur);
	}

	ThreadYield();
}

static Thread* AllocateThread()
{
    static int n = 1;
    Thread* t;

    if ((t = (Thread*)malloc(sizeof(Thread))) == NULL) {
	DEBUG("*** error: fail to allocate a thread");
	exit(1);	/* error: no memory */
    }

    t->next = NULL;
    t->thread_id = n++;
    t->stack_top = NULL;
    t->status = RUNNING;
    return t;
}

static void FreeThread(Thread* t)
{
    free(t->stack_top);
    free(t);
}
