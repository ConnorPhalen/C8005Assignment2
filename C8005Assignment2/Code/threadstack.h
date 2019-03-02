#ifndef THREADSTACK_H_
#define THREADSTACK_H_
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
// Struct Declaration
#define STACKLIMIT 10
struct ThreadStack // Thread Stack for holding pointers to waiting pthreads
{
	pthread_t *stack[STACKLIMIT];	// array of thread pointers
	int top; 						// current index of the top thread, starts at 0
};
struct tnode // Thread Node for allocating and removing threads (A.K.A. Linked Lists)
{
	pthread_t thread;	// Actual thread
	int *clientsock;	// socket that the above thread will use
	bool joinable;
	struct tnode *prev;	// Pointer to previous node
	struct tnode *next;	// Pointer to next node
};
// Function Declaration
pthread_t* stackpop(struct ThreadStack *tstack);
int stackpush(pthread_t *thread, struct ThreadStack *tstack);
int stackisfull(struct ThreadStack *tstack);
struct tnode * tnodermt(struct tnode *headnode, pthread_t dthread);
struct tnode * tnodepop(struct tnode *headnode, struct tnode *popnode);
struct tnode * tnodepush(struct tnode *tailnode, struct tnode *pushnode);
#endif // THREADSTACK_H_