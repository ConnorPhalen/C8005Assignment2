/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	client2.c
--
--	PROGRAM:		client2
--
--	FUNCTIONS:		Add Additional Functions Used Here
--
--	DATE:			February 11, 2019
--
--	REVISIONS:		(Date and Description)
--
--              February 11, 2019:
--                  - Initial Setup and Push to GitHub Repo
--              February 24, 2019:
--                  - Version 2
--
--	DESIGNERS:		Connor Phalen and Greg Little
--
--	PROGRAMMERS:	Connor Phalen and Greg Little
--
--	NOTES:
--	Compile using this -> gcc -Wall client2.c -o client2 -lpthread -g
--  Run using -> ./client2 [ip address] [num of clients] [f or p] 
--  Note f = file transfer & p = send user input paragraph
---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>


#define SERVER_LISTEN_PORT 8080
#define BUFLEN 1024
#define THREAD_INIT 1000000
struct targs{
    char* host;
    char* work;
};

int count;
void* connectionWorker (void *arguments);

pthread_mutex_t lock; // Global mutex

// Program Start
int main(int argc, char **argv)
{
    int number_client = 1;
    int waitTime = 25;
    count = 0;
    pthread_t threads[THREAD_INIT];//make linked list

//	FILE *filereader; // File Descriptors for saving performance
//	FILE *filewriter;  // for later implementation

	char  *host;
    char  *work;
	switch(argc)
	{
		case 4:
			host = argv[1];
            number_client = strtol(argv[2],NULL,10);
            work = argv[3];
			break;
		default:
			fprintf(stderr, "Usage: %s Enter a host ip, then number of clients, then f/p", argv[0]);
			exit(1);
	}
    if(number_client / 10 < waitTime){
      waitTime = number_client/10;
    }
    printf("num: %d\n",number_client);
	// Create new socket;
    //sending socket_desc to thread
    if(strcmp(work,"p")==0){
        printf("enter text to send: ");
        fgets(work,BUFLEN,stdin);
    }

    struct targs args = (struct targs){host,work};

    //init mutex
    if(pthread_mutex_init(&lock, NULL) != 0){
        perror("Error initializing mutex\n");
        exit(1);
    }

    //creating threads for clients
    for(int i=0; i<number_client;i++){
        if(pthread_create(&threads[i],NULL,&connectionWorker,(void *)&args) !=0){
            printf("Error Occured with Thread Creation");
            return -1;
        }
        //if(i%10==0){
            usleep(7000);
        //}
        printf("thread number: %d\n", count);
        count++;

    }
    time_t timer = time(0) + waitTime;
    while(time(0) < timer){} // compiler is being a child about this cant use ;

    // pthread_mutex_destroy(&lock); // Use when all threads are done
	return(0);
}


void* connectionWorker (void *arguments){
    char  **pptr;
	char str[16];
    int socket_desc;
    struct hostent	*hp;
    struct sockaddr_in server;
    struct targs *args = arguments;
    char *host = args->host;
    char *work = args->work;
    int waitTime =  2, bytes_to_read, n;
    char *bp;
    char send_buf[BUFLEN], recieve_buf[BUFLEN];

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
    }else{

    //printf("Connected to Server: %s ", hp->h_name);
    pptr = hp->h_addr_list;
    printf("IP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
    //printf("Send a Message to the server: \n");
    }

        for(int i=0; i<3; i++){
            time_t timer = time(0) + waitTime;
            if(strcmp(work,"f")==0){
                char message[5] = {"\0"};
                strcat(message,"x.txt");
                message[0] = i+'0';
                //printf("message %s\n", message);
                FILE *send_txt = fopen(message,"r");
                fgets(send_buf,BUFLEN,send_txt);
                fclose (send_txt);
            }else{
                strcpy(send_buf,work);
            }
            //printf("%s\n",send_buf);
    		send(socket_desc, send_buf, BUFLEN, 0);

    		bp = recieve_buf;
    		bytes_to_read = BUFLEN;

    		bzero(bp, BUFLEN);
            n=0;
    		// Keep receiving until no more data on socket
    //while(strcmp(send_buf,recieve_buf)!=0)
    		while((n = recv(socket_desc, bp, bytes_to_read, 0)) < bytes_to_read)//change bytes_to_read
    		{
    			bp += n;
    			bytes_to_read -= n;
    		}

    		//printf("Message Recieved From Server: \n");
    		//printf("%s\n", recieve_buf);
            fflush(stdout);
            bzero(send_buf, BUFLEN);
            while(time(0) < timer);
        }
        // Clear stdout
        fflush(stdout);
        //close socket
        close(socket_desc);
        //pthread_exit(0);
        return 0;
}
