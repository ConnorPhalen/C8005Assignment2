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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "threadstack.h"

pthread_t* stackpop(struct ThreadStack *tstack)
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
int stackpush(pthread_t *thread, struct ThreadStack *tstack)
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
int stackisfull(struct ThreadStack *tstack)
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

/* tnode_rm_thread - remove a node from a linked list that has the corresponding thread
*  Input 	- headnode - Head of the linked list | dthread - pointer toa thread we want gone
*  Output	- returns a pointer to the removed tnode so we can free its memory, or NULL if thread does not exist
*  NOTE: Atlmost the same as tnode_pop, except that it is intended to be used with a thread that returned from its tour of duty
*/
struct tnode * tnodermt(struct tnode *headnode, pthread_t *dthread)
{
	struct tnode *dnode; // node which we will use to move around, then return at proper node

	for(dnode = headnode; pthread_equal(*dthread, dnode->thread) != 0; dnode = dnode->next) // for every node we come across that matches the thread ID's
	{
		if(dnode->prev == NULL)	// check if head
		{
			dnode->next->prev = NULL;
			return dnode; // dnode was head all along
		}
		if(dnode->next == NULL)	// check if tail
		{
			dnode->prev->next = NULL;
			return dnode; // dnode was tail all along
		}
		// reassign next and previous pointers
		dnode->prev->next = dnode->next;
		dnode->next->prev  = dnode->prev;
		return dnode;
	}
	return NULL; // That thread ID does not exist
}

/* tnode_pop - remove a node from a linked list by reassinging the next & prev nodes
*  Input 	 - headnode - Head of the linked list | popnode - tnode to pop out of list
*  Output	 - returns a pointer to be popped out
*/
struct tnode * tnodepop(struct tnode *headnode, struct tnode *popnode)
{
	struct tnode *dnode; // node which we will use to move around, then return at proper node

	for(dnode = headnode; pthread_equal(popnode->thread, dnode->thread) != 0; dnode = dnode->next) // for every node we come across that matches the thread ID's
	{
		if(dnode->prev == NULL)	// check if head
		{
			//if(dnode->next == NULL){} // Is head and tail node
			
			dnode->next->prev = NULL;
			return dnode; // dnode was head all along
		}
		if(dnode->next == NULL)	// check if tail
		{
			//if(dnode->prev == NULL){} // Is tail and head node

			dnode->prev->next = NULL;
			return dnode; // dnode was tail all along
		}
		// reassign next and previous pointers
		dnode->prev->next = dnode->next;
		dnode->next->prev  = dnode->prev;
		return dnode;
	}
	return NULL; // That thread ID does not exist
}

/* tnode_push - pusha  new node at the end of the linked list
*  Input 	  - tail - Tail of the linked list | pushnode - tnode to push into list
*  Output	  - returns a pointer to the pushed node
*  NOTE: The pushed node will become the new tail node when returned
*/
struct tnode * tnodepush(struct tnode *tail, struct tnode *pushnode)
{
	if(tail == NULL) // must be the first push
	{
		pushnode->next = pushnode; // make next equal to itself???????????????
		return pushnode; // return pointer to pushed node
	}
	tail->next = pushnode;
	pushnode->prev = tail; // could be head node as well, but this assignment wouldn't matter
	pushnode->next = NULL; 
	//tail = pushnode; // Here as a reminded of pass-by-value... for some reason

	return pushnode; // return pointer to pushed node
}