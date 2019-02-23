#ifndef THREADSTACK_H_
#define THREADSTACK_H_
#include <stdlib.h>
#include <pthread.h>
// Struct Declaration
#define STACKLIMIT 10
struct ThreadStack
{
	pthread_t *stack[STACKLIMIT];	// array of thread pointers
	int top; 						// current index of the top thread, starts at 0
};
// Function Declaration
pthread_t* pop(struct ThreadStack *tstack);
int push(pthread_t *thread, struct ThreadStack *tstack);
int isfull(struct ThreadStack *tstack);
#endif // THREADSTACK_H_