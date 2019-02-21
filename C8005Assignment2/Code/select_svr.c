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
--	Compile using this -> gcc -Wall -o sel_svr select_svr.c 
--  https://stackoverflow.com/questions/26753957/how-to-dynamically-allocateinitialize-a-pthread-array
---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define BUFLEN 255
#define LISTEN_QUOTA 5
#define THREAD_INIT 10

/* ---- Function Prototypes ---- */
int tconnect(int yes, char* no);

// Program Start
int main(int argc, char **argv)
{
/* ---- Variable Setup ---- */
	int i, maxi, nready, bytes_to_read;
	int socket_desc, new_socket_desc, client_len; 	// Socket specific
	int port, maxfd, client[FD_SETSIZE];			// Select specific
	ssize_t n;

	pthread_t tlist[THREAD_INIT];	// Create list of threads, this will top up with unused threads

	char *bp, buf[BUFLEN];

   	fd_set rset, allset; 	// Select variables for file descriptors
	FILE *filewriter; 		// For exporting performance data

	struct sockaddr_in server, client;

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
   	maxi	= -1;			// index into client[] array

	for (i = 0; i < FD_SETSIZE; i++) // Loop to set all entries to -1 (no connections)
	{
           	client[i] = -1;            
	}
 	FD_ZERO(&allset);				// zero out allset
   	FD_SET(socket_desc, &allset);	// set allset to be used with socket_desc 

	while(TRUE)
	{









		/*
		client_len = sizeof(client);

		// Set new socket to accept client connection
		if((new_socket_desc = accept(socket_desc, (struct sockaddr *)&client, &client_len)) == -1)
		{
			fprintf(stderr, "Failed to connect to new client\n");
			exit(1);
		}

		printf("Remote Address: %s\n", inet_ntoa(client.sin_addr));
		bp = buf;
		bytes_to_read = BUFLEN; // Read BUFLEN bytes in socket stream

		// Read in bytes as long as there are bytes to read
		while((n = recv(new_socket_desc, bp, bytes_to_read, 0)) < BUFLEN)
		{
			bp += n;
			bytes_to_read -= n;
		}

		printf("User has sent over the message: %s\n", buf);

		bzero(bp, BUFLEN);

		if(send(socket_desc, bp, BUFLEN, 0) < 0)
		{
			fprintf(stderr, "Failed to send File: %s\n", buf);
			exit(1);
		}

		// Close Listening socket
		close(socket_desc);
		return(0);
		*/
		
	}
/* ---- Closing Tasks ---- */
	// Close Listening socket
	close(socket_desc);
	return(0);
}


// tconnect: Thread Process for client connection and data processing
// Input  - 
// Output - 
int tconnect(int yes, char* no);
{

}