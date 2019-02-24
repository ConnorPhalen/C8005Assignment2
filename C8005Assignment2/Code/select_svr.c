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
--  FD_SETSIZE is restricted to 1024, so that is the select servers upper limit. Could extend it by making another higher variable?
--	Compile using this -> gcc -Wall -o sel_svr select_svr.c threadstack.c -pthread
--  https://stackoverflow.com/questions/26753957/how-to-dynamically-allocateinitialize-a-pthread-array
---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>
#include "threadstack.h"

#define SERVER_PORT 8080
#define BUFLEN 255
#define LISTEN_QUOTA 5
#define THREAD_INIT 10
#define SLEEPYTIME 10 	// Note: microseconds

/* ---- Function Prototypes ---- */
void* tprocess(void *arguments);

struct targs{
	int *clientsock;
	fd_set *readset;
	fd_set *allset;
};

// Program Start
int main(int argc, char **argv)
{
/* ---- Variable Setup ---- */
	int i, maxi, nready, stacksize;
	int socket_desc, new_socket_desc, client_len; 	// Socket specific
	int port, maxfd, clientfd[FD_SETSIZE];			// Select specific
	struct sockaddr_in server, client;

	pthread_t tlist[THREAD_INIT];	// Create list of threads, this will top up with unused threads

	// pthread_mutex_t mutex; // Look into possibly using this. Too many active threads using one mutex may just make everything block because it will be a valuable resource

	// char *bp, buf[BUFLEN]; // Moved to thread
	// int bytes_to_read;
	// ssize_t n;

   	fd_set readset, allset; 	// Select variables for file descriptors
	FILE *filewriter; 		// For exporting performance data

	struct timeval timeout = (struct timeval){ 1 };	// timeout of 1 second

	struct ThreadStack tstack;	// struct for popping and pulling

/* ---- Socket Init ---- */
	fprintf(stdout, "Opening server on Port %d\n", SERVER_PORT);

	// Create socket
	if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		// Error in Socket creation
		perror("Socket failed to be created");
		exit(1);
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

	for (i = 0; i < FD_SETSIZE; i++) // Loop to set all entries to -1 (no connections)
	{
           	clientfd[i] = -1;            
	}
 	FD_ZERO(&allset);				// zero out allset
   	FD_SET(socket_desc, &allset);	// set allset to be used with socket_desc 

	while(true) // select calls for client connections and thread creation/management
	{
		timeout.tv_sec = 1; // reset timeout as select might modify it

   		readset = allset;              
   		// Blocks here until timeout or readfds || writedfs || exceptfds crit
		nready = select(maxfd + 1, &readset, NULL, NULL, &timeout); 

		switch(nready)
		{
			case 0: // Timeout case
				stacksize = isfull(&tstack);
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

/* ---- New Client Has Connected ---- */
		if(FD_ISSET(socket_desc, &readset)) // New file descriptor set, means new client wants connect
		{
			client_len = sizeof(client);
			// accept new client 
			if((new_socket_desc = accept(socket_desc, (struct sockaddr *) &client, &client_len)) == -1)
			{
				perror("Error accepting new client");
			}
			printf("New Client Address:  %s\n", inet_ntoa(client.sin_addr));

	        for(i = 0; i < FD_SETSIZE; i++) // Look for open file desc for new client
	        {
				if(clientfd[i] < 0) // Empty file desc found
	            {
					clientfd[i] = new_socket_desc;	// save descriptor

					// TODO: HAND OFF NEW CLIENT CONNECTION TO NEW THREAD

					struct targs args = (struct targs){&new_socket_desc, &readset, &allset}; // worried about memory leaking from this once thread closes

				    if(pthread_create(&tlist[0], NULL, &tprocess, (void *)&args) != 0) 
				    {
				        printf("Error Occured with Thread Creation");
				        return -1;
				    }

					break;
	            }
				if(i == FD_SETSIZE) // No empty space for new clients (1024 clients????)
	         	{
					printf("Too many clients\n");
	            			exit(1);
	    		}

				FD_SET(new_socket_desc, &allset);     // add new descriptor to set

				if(new_socket_desc > maxfd)
				{
					maxfd = new_socket_desc;	// for select
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
	// Close Listening socket
	close(socket_desc);
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

	struct targs *targs = arguments;

	int *clientsock = targs->clientsock; // Feel like this is a waste of memory :(
	fd_set *readset = targs->readset;
	fd_set *allset  = targs->allset;

	// Write this to performance file? Pipe back process for that?
	printf("Hello, my name is Thread #%ld, it is a pleasure to do business with you :)\n", pthread_self());

	while(true)
	{
		/*
		if(clientsock < 0) // if client descriptor has no data, wait
		{
			continue;
		}
		*/
/* ---- Check for data ---- */
		if(FD_ISSET(*clientsock, &(*readset))) // Check to see if our socket is flagged for reading
		{
			bp = buf;
			bytes_to_read = BUFLEN;
			while ((n = read(*clientsock, bp, bytes_to_read)) > 0) 
			{
				bp += n;
				bytes_to_read -= n;
			}
			write(*clientsock, buf, BUFLEN);   // echo to client
		}

/* ---- Communicate with Main Process for this Threads Closure ---- */
		if (n == 0) // connection closed  NOTE: Have a timeout possible as well for unexpected client closure
	    {
			printf("Client has closed connection. Starting thread closure process...\n");
			close(*clientsock);
			FD_CLR(*clientsock, allset);

			// Communicate with main process for last bit of cleanup
	       	// client[i] = -1;

	       	// Thread Join or what for closing it???? Cand is it called from process????
			return NULL;
	    }
	}
	return NULL;
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