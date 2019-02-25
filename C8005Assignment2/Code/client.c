/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	client.c
--
--	PROGRAM:		clnt
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
--	Compile using this -> gcc -Wall -o clnt client.c
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
#include <time.h>


#define SERVER_LISTEN_PORT 8080
#define BUFLEN 1024

// Program Start
int main(int argc, char **argv)
{
    int waitTime =  5;
	int bytes_to_read, n;
	int socket_desc;

	struct hostent	*hp;
	struct sockaddr_in server;
	struct stat buffer;

	FILE *filereader; // File Descriptors for saving performance
	FILE *filewriter;

	char  *host, *bp, **pptr;
	char str[16], send_buf[BUFLEN], recieve_buf[BUFLEN];

	switch(argc)
	{
		case 2:
			host = argv[1];
			break;
		default:
			fprintf(stderr, "Usage: %s Enter a host ip", argv[0]);
			exit(1);
	}

	// Create new socket
	if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}
	// Zero out and create memory
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_LISTEN_PORT); // Flip those connection bits

	// Get name of host
	if((hp = gethostbyname(host)) ==  NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	if(connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Failed to connect to server\n");
		perror("connect");
		exit(1);
	}

	printf("Connected to Server: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	printf("Send a Message to the server: \n");

    for(int i=0; i<3; i++){
        char message[5] = {"\0"};
        time_t timer = time(0) + waitTime;

        strcat(message,"x.txt");
        message[0] = i+'0';
        printf("message %s\n", message);
        FILE *send_txt = fopen(message,"r");
        fgets(send_buf,BUFLEN,send_txt);
        printf("%s\n",send_buf);
		send(socket_desc, send_buf, BUFLEN, 0);

		bp = recieve_buf;
		bytes_to_read = BUFLEN;

		bzero(bp, BUFLEN);
		int recv_block = 0;

		// Keep receiving until no more data on socket
		while((recv_block = recv(socket_desc, bp, bytes_to_read, 0)) < bytes_to_read)
		{
			bp += n;
			bytes_to_read -= n;
		}

		printf("Message Recieved From Server: \n");
		printf("%s\n", recieve_buf);

        memset(send_buf, '\0', BUFLEN);

        fclose (send_txt);
        while(time(0) < timer);
    }
/*
	// Get user's text from standard input
	fgets(send_buf, BUFLEN, stdin);

	// Send request to the server
	send(socket_desc, send_buf, BUFLEN, 0);

	bp = recieve_buf;
	bytes_to_read = BUFLEN;

	bzero(bp, BUFLEN);
	int recv_block = 0;

	// Keep receiving until no more data on socket
	while((recv_block = recv(socket_desc, bp, bytes_to_read, 0)) > 0)
	{
		bp += n;
		bytes_to_read -= n;
	}

	printf("Message Recieved From Server: \n");
	printf("%s\n", recieve_buf);
*/
	// Clear stdout
	fflush(stdout);
	close(socket_desc);
	return(0);
}
