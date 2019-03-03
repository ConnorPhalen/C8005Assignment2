/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	select_svr.c 
--
--	PROGRAM:		sel_svr
--
--	FUNCTIONS:		Add Additional Functions Used Here 
--
--	DATE:			February 11, 2019
--
--	REVISIONS:		(Date and Description)
--
--				February 11, 2019: 
--					- Initial Setup and Push to GitHub Repo
--
--	DESIGNERS:		Connor Phalen and Greg Little
--
--	PROGRAMMERS:	Connor Phalen and Greg Little
--
--	NOTES:
--	Compile using this -> gcc -Wall -o sel_svr select_svr.c threadstack.c -lpthread -g
--  https://stackoverflow.com/questions/911860/does-malloc-lazily-create-the-backing-pages-for-an-allocation-on-linux-and-othe
--  https://stackoverflow.com/questions/26753957/how-to-dynamically-allocateinitialize-a-pthread-array
---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include "threadstack.h"

#define SERVER_PORT 8080
#define BUFLEN 1024
#define LISTEN_QUOTA 5
#define THREAD_INIT 1000000
#define THREAD_TIMEOUT 3
#define FD_SETSIZE2 20000

/* ---- Function Prototypes ---- */
void* tprocess(void *arguments);

struct targs{
	int *clientsock;
	struct tnode *nodehold;
	FILE *perfwriter;
	fd_set *readset;
	fd_set *allset;
	// Do we need sockaddr_in client for anything?
};

pthread_mutex_t tlock; // Global mutex for threads
pthread_mutex_t memlock; // Global mutex for memory creation
pthread_mutex_t filelock; // Global mutex for memory creation

// Program Start
int main(int argc, char **argv)
{
/* ---- Variable Setup ---- */
	int i, maxi, nready, conncounter;
	int socket_desc, client_len; 	// Socket specific
	int sockpoint;
	int maxfd, clientfd[FD_SETSIZE2];			// Select specific
	struct sockaddr_in server, client;

	struct tnode *head, *tail, *dnode; // pointers to the head and tail nodes

   	fd_set readset, allset; // Select variables for file descriptors
	FILE *filewriter; 		// For exporting performance data

	struct timeval timeout = (struct timeval){ 1 };	// timeout of 1 second

/* ---- Variable Testing  ---- */
	// init memory for node, malloc might be faster, but am worried about its "optimistic memory allocation" (see notes)
	struct tnode *yesnode 	= calloc(1, sizeof(struct tnode));
	struct tnode *nonode 	= calloc(1, sizeof(struct tnode));

	head = yesnode;
	tail = NULL;

	tail = tnodepush(tail, yesnode);
	tail = tnodepush(tail, nonode);

/* ---- Socket Init ---- */
	fprintf(stdout, "Opening server on Port %d\n", SERVER_PORT);

	// Create socket
	if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		// Error in Socket creation
		perror("Socket failed to be created");
		exit(1);
	}

	if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
	{
    	perror("setsockopt(SO_REUSEADDR) failed");
	}

	// Zero out and create memory for the server struct
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);  // Flip the bits for the weird intel chip processing
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept addresses from anybody

	// Bind new socket to port
	if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Cannot bind to socket");
		exit(1);
	}

/* ---- New Connection And Thread Setup ---- */
	// Listen for Connections, limit to backlog of LISTEN_QUOTA requests
	listen(socket_desc, LISTEN_QUOTA);

	maxfd	= socket_desc;	// initialize
   	maxi	= -1;			// index into clientfd[] array

	for (i = 0; i < FD_SETSIZE2; i++) // Loop to set all entries to -1 (no connections)
	{
           	clientfd[i] = -1;            
	}
 	FD_ZERO(&allset);				// zero out allset
   	FD_SET(socket_desc, &allset);	// set allset to be used with socket_desc 

   	if(pthread_mutex_init(&tlock, NULL) != 0)
   	{
   		perror("Error initializing thread mutex");
   		exit(1);
   	}
   	if(pthread_mutex_init(&memlock, NULL) != 0)
   	{
   		perror("Error initializing thread mutex");
   		exit(1);
   	}
   	if(pthread_mutex_init(&filelock, NULL) != 0)
   	{
   		perror("Error initializing thread mutex");
   		exit(1);
   	}

   	signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE errors, will handle the errors when they happen

   	/*
   	* New Thread Setup that manages the closing funcs of other threads.
   	* 
   	*/

	while(true) // select calls for client connections and thread creation/management
	{
		timeout.tv_sec = (size_t)1; // reset timeout as select might modify it

   		readset = allset;              
   		// Blocks here until timeout or readfds || writedfs || exceptfds crit
		nready = select(maxfd + 1, &readset, NULL, NULL, &timeout); 

/* ---- Thread and TNode Cleanup ---- */
		dnode = head;
		while(dnode->next != NULL)
		{
			// printf("smol check -----");
			if(dnode->joinable == true)	// Should we mutex the clientfd[x] = -1; ????
			{
				printf("Closing Thread #%ld\n", dnode->thread);
				int result;
				if((result = pthread_join(dnode->thread, NULL)) != 0)
				{
					printf("Error Joining Threads: %s\n", strerror(result));
					exit(1);
				}
				FD_CLR(*(dnode->clientsock), &allset);
				close(*(dnode->clientsock));

				for(i = 0; i < FD_SETSIZE2; i++) // reset clientfd spot for this socket
				{
					if(clientfd[i] == *(dnode->clientsock))
					{
						clientfd[i] = -1;
						break;
					}
				}
				free(tnodepop(head, dnode));
			}
			dnode = dnode->next;
		}
		//printf(" ---- \n");
/*
		switch(nready)
		{
			case 0: // Timeout case
				stacksize = stackisfull(&tstack);
				if(stacksize > 0)
				{
					// Stack is not full, so make new threads and top up stack
					// Make function for new thread creation? We will use that code in another part of this program
				}
				// printf("Yeppers");
				continue;
			case -1: // Error Case
				// Select Error occured, can process specific errors below
				perror("Error reading select descriptors");
				exit(1);
			default:
				// returns the number of file descriptors in the select descriptor sets
				break;
		}
*/
/* ---- New Client Has Connected ---- */
		if(FD_ISSET(socket_desc, &readset)) // New file descriptor set, means new client wants connect
		{
			client_len = sizeof(client);

			// accept new client 
			if((sockpoint = accept(socket_desc, (struct sockaddr *) &client, &client_len)) == -1)
			{
				perror("Error accepting new client");
				exit(1);
			}
			// printf("New Client Address:  %s\n", inet_ntoa(client.sin_addr));

	        for(i = 0; i < FD_SETSIZE2; i++) // Look for open file desc for new client
	        {
				if(clientfd[i] < 0) // Empty file desc found
	            {
					clientfd[i] = sockpoint;	// save descriptor

					struct tnode *threadnode = calloc(1, sizeof(struct tnode)); 
					struct targs *args 		 = calloc(1, sizeof(struct targs));

					// struct targs args = (struct targs){&(clientfd[i]), threadnode, &readset, &allset}; // worried about memory leaking from this once thread closes
					args->clientsock = &(clientfd[i]);
					args->nodehold 	 = threadnode;
					args->perfwriter = filewriter;
					args->readset 	 = &readset;
					args->allset 	 = &allset;
					threadnode->clientsock = &(clientfd[i]);

					//printf("Starting Thread - %d\n", i);
				    if(pthread_create(&(threadnode->thread), NULL, &tprocess, (void *)args) != 0) // !!!! FIX THREAD INIT !!!!
				    {
				    	if(errno == EAGAIN) // System Resources are strained
				    	{
				        	perror("Error Occured with Thread Creation");
				        	exit(1);		    		
				    	}
				    }
				    tail = tnodepush(tail, threadnode); // Push new thread into linked list
					break;
	            }
				if(i == FD_SETSIZE2) // No empty space for new clients (1024 clients????)
	         	{
					printf("Too many clients\n\n\n");
	            	exit(1);
	    		}

				FD_SET(sockpoint, &allset);     // add new descriptor to set

				if(sockpoint > maxfd)
				{
					maxfd = sockpoint;	// for select
				}
				if(i > maxi) // maxi set to highest client index
				{
					maxi = i;
				}
				if(--nready <= 0) // No suitable file descriptors?
				{
					continue;	      	
				}
	        }
		}

	}
/* ---- Closing Tasks ---- */
	// Close these by accepting the CRTL+C input and directing here
	//free(tail);
	//free(head);
	close(socket_desc);
	pthread_mutex_destroy(&tlock);
	pthread_mutex_destroy(&memlock);
	return(0);
}


// tprocess: Thread Process for client connection and data processing
// Input  - clientsock: Client Socket Descriptor | fd_set: Pointer to main readset | allset: pointer to allset
// Output - 
void* tprocess(void *arguments)
{
	char *bp, buf[BUFLEN];
	int bytes_to_read;
	ssize_t n = -1;

	//struct targs *targs = arguments; // Can we skip to just using the arguments pointer below here?
	//struct timeval ttimeout_old, ttimeout_new;// Setup timeout variable

	pthread_mutex_lock(&memlock);
	struct targs *targs = arguments;
	pthread_mutex_unlock(&memlock);
//	pthread_mutex_lock(&memlock);
//	struct targs *targs = calloc(1, sizeof(struct targs));
//	if(memcpy((struct targs *)targs, arguments, sizeof(struct targs)) == NULL)
//	{
//		perror("Failed to allocate memory for thread arguments");
//		exit(1);
//	}
//	pthread_mutex_unlock(&memlock);

	//int *clientsock = targs->clientsock;
	fd_set *readset = targs->readset;
	fd_set *allset  = targs->allset;

	// Write this to performance file? Pipe back process for that?
	//printf("Thread #%ld reporting in, it is a pleasure to do business with you :)\n\n", pthread_self());

	//gettimeofday(&ttimeout_old, NULL); // get time of day for later

	while(true)
	{
/* ---- Check for data ---- */
		if(FD_ISSET(*(targs->clientsock), readset)) // Check to see if our socket is flagged for reading
		{
			bp = buf;
			bytes_to_read = BUFLEN;
			while ((n = read(*(targs->clientsock), bp, bytes_to_read)) < bytes_to_read) 
			{
				bp += n;
				bytes_to_read -= n;
				pthread_mutex_lock(&tlock);
				printf("Socket -> %d to Thread ID %ld\n", *(targs->clientsock), pthread_self());
				pthread_mutex_unlock(&tlock);
				if(n == 0)
				{
					break; // no more bytes to read
				}

//				if(n == -1)
//				{
//					perror("ERROR"); // Connection reset by peer (client seg fault)
//					exit(1);
//				}
			}
			pthread_mutex_lock(&tlock); // Bad? but fixes multiple writes that cause borked pipes
			if(write(*(targs->clientsock), buf, BUFLEN) == -1)   // echo to client
			{
				if(errno == EPIPE) // If Peer end was broken
				{
					break; // break out so we can clsoe connection
				}
				/* Deal with other Client Pipe errors here */
				perror("Error writing to client");
				printf("THREAD - %ld\nSOCKET - %d\n", pthread_self(), *(targs->clientsock));
				exit(1);
			}
			pthread_mutex_unlock(&tlock);
			//gettimeofday(&ttimeout_old, NULL); // active connection, so update ttimeout_old
		}

		//gettimeofday(&ttimeout_new, NULL); // update time, probably slow

/* ---- Communicate with Main Process for this Threads Closure ---- */
		// if((ttimeout_new.tv_sec - ttimeout_old.tv_sec) > THREAD_TIMEOUT)
		if(n == 0) // check if timeout reached
	    {
	    	break;
	    }
	}
	pthread_mutex_lock(&tlock);
	printf("Thread #%ld: Client done, closing thread...\n", pthread_self());
	FD_CLR(*(targs->clientsock), allset); // move to cleanup section????

   	targs->nodehold->joinable = true; // set tnode to specify its thread can be joined
   	free(arguments);
   	pthread_mutex_unlock(&tlock);
    //pthread_exit(NULL);
    return 0;
}


/* Thread Notes:
Option A - 	Have a Thread struct that holds the data they need to operate. 
			Threads are made and wait until the main process sends them the proper data.
			They recieve the data (which is client connection info and location in readset and allset)
			then the threads will spin and handle the data and reading. Would be nice as we already
			need a thread argument struct.

Option B - 	Have basic Thread variables, then pass in a function for them to run with the parameters
			being the client and readset and allset data they need to operate. The threads will
			then spin and handle reading and handling client data. Would need to use the thread
			argument struct that is prototyped at the start.

A vs. B - 	"A" might be better for performace as all the variables and stuff is allocated
			before client connection. Only need to assign that data and the thread is ready to go.
			Custom Thread struct could help with adding special stuff to it later.
			"B" is much simpler and doesn't have to have more overhead of waiting for a pipe/what-not
			from the main process. 
*/