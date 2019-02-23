/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	threadstack.c 
--
--	PROGRAM:		N/A
--
--	FUNCTIONS:		Add Additional Functions Used Here 
--
--	DATE:			February 22, 2019
--
--	REVISIONS:		(Date and Description)
--
--				February 22, 2019: 
--					- threadstack structure implementation
--
--	DESIGNERS:		Connor Phalen and Greg Little
--
--	PROGRAMMERS:	Connor Phalen and Greg Little
--
--	NOTES:
--  Makes a stack structure to hold threads. Pops threads for connecting, and pushes threads to wait
---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "threadstack.h"

pthread_t* pop(struct ThreadStack *tstack)
{
	pthread_t *threadp;
	if(tstack->top <= 0)	// if top is negative or at first position (empty)
	{
		return NULL;
	}
	else
	{
		tstack->top--;
		threadp = tstack->stack[tstack->top]; // assign the pointers
		return threadp;
	}
}

/* push 	- pushes a thread onto the stack
*  Input 	- takes in a pointer to a pthread_t to put onto the stack
*  Output	- returns the position of the pushed thread, -1 if full, -2 if error
*/
int push(pthread_t *thread, struct ThreadStack *tstack)
{
	if(tstack->top >= STACKLIMIT || tstack->top < 0) // If stack is inbounds
	{
		return -1; // send negative number back to signal full
	}
	else
	{
		tstack->stack[tstack->top] = thread;	// push thread onto stack
		tstack->top++; 					// increment top position
	}

	return tstack->top;	// return top position, other end can ignore, or make an assumption based off this
}

/* isfull 	- checkes to see if stack is topped off
*  Input 	- N/A
*  Output	- returns -1 if full, or position of the top thread
*/
int isfull(struct ThreadStack *tstack)
{
	if(tstack->top >= STACKLIMIT)
	{
		return -1;
	}
	else
	{
		return tstack->top;
	}
}