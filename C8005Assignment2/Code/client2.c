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
--	Compile using this -> gcc -Wall -o client client2.c -lpthread
---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_LISTEN_PORT 8080
#define PERFFILE "client_perf.csv"
#define BUFLEN 1024
#define PROCMOD 4 // Adjusts spacing between sends, higher numbers is higher delay

struct targs{
    char* host;
    char* work;
};


int connectionWorker (char* host, char* work);
void closeProc(int sigNum);
double delay(struct timeval *start, struct timeval *end);

int send_amount = PROCMOD; // The higher the number, the more overlap between sleep sections, can be local if we delete thread function
pthread_mutex_t *filelock; // might not work as is with multi-processing. Oh well, the program works in general

// Program Start
int main(int argc, char **argv)
{
    int number_client = 1;
    char  *host;
    char  work[BUFLEN];

    FILE *filewriter;
    struct timeval tstart, tcheck;

    //worker
    int socket_desc;
    struct hostent	*hp;
    struct sockaddr_in server;
    int waitTime =  2, bytes_to_read, n;
    char *bp;
    
	switch(argc)
	{
        case 3:
            host = argv[1];
            number_client = strtol(argv[2],NULL,10);
            break;
        case 4:
            host = argv[1];
            number_client = strtol(argv[2],NULL,10);
            send_amount = strtol(argv[3],NULL,10);
            break;
		default:
			fprintf(stderr, "Usage: %s [hostip] [number of clients] (Optional Flags: [times to send])\n", argv[0]);
            exit(1);
	}

    char send_buf[BUFLEN], recieve_buf[BUFLEN];

    if(number_client / 5 < waitTime){
      waitTime = number_client/5;
    }
    printf("num: %d\n",number_client);
	// Create new socket;
    //sending socket_desc to thread
    //if(strcmp(work,"p")==0){
    printf("enter text to send: ");
    fgets(work,BUFLEN,stdin);
    work[strlen(work)-1] = '\0';
    //}

    filelock = calloc(1, sizeof(pthread_mutex_t));

    if(pthread_mutex_init(*(&filelock), NULL) != 0)
    {
        perror("Error initializing thread mutex");
        exit(1);
    }
    gettimeofday(&tstart, NULL);

    if((filewriter = fopen(PERFFILE, "w+")) == NULL) //clear old file if it exists, will get appened to later
    {
        perror("Failed to open file");
        exit(1);
    }
    fclose(filewriter);

    signal(SIGINT, closeProc); // Just to guarantee the forked proc is closed with Ctrl+C

    pid_t forker;

    for (int i=1; i< number_client; i++){
        if((forker = fork()) == 0){
          break;
        }else if(forker<0){
            i--;
        }
    }

    //connectionWorker
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
        //  pptr = hp->h_addr_list;
        //    printf("IP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
        //printf("Send a Message to the server: \n");
    }

    // mod the id, and sleep based on that amount, helps seperate sends and stuff
    usleep((getpid() % PROCMOD) * (PROCMOD * PROCMOD)); // might want to adjsut to get more sends later than evenly as is now

    for(int i=0; i<send_amount; i++){
        strcpy(send_buf,work);

        send(socket_desc, send_buf, BUFLEN, 0);

    /* ---- PROCESS SYNCING ---- */
        pthread_mutex_lock(*(&filelock)); // might work, might not. Lets find out

        if((filewriter = fopen(PERFFILE, "a")) == NULL) // open up client file to append data
        {
            perror("Failed to open file");
            exit(1);
        }
        gettimeofday(&tcheck, NULL);

        double delayed = delay(&tstart, &tcheck);
        printf("Time Stuff - %.0f & ", (delayed));
        fprintf(filewriter," %.2f | %d,\n", delayed, BUFLEN);

        fclose(filewriter);
        gettimeofday(&tstart, NULL); // update tstart to reflect that it was jsut logged

        pthread_mutex_unlock(*(&filelock));

        bp = recieve_buf;
        bytes_to_read = BUFLEN;

        bzero(bp, BUFLEN);
            n=0;
        // Keep receiving until no more data on socket
        while((n = recv(socket_desc, bp, bytes_to_read, 0)) < bytes_to_read)//change bytes_to_read
        {
          bp += n;
          bytes_to_read -= n;
        }

        //printf("Message Recieved From Server - R:%s\n", recieve_buf);
    }
    // Clear stdout
    fflush(stdout);
    //close socket
    close(socket_desc);

    int status = 0;
    if(forker==0){
        //printf("hello");
        exit(0);
        while((forker=wait(&status))>0); // will make only main process wait for X time, could make signal based instead
    }
    printf("its over\n");
    free(filelock);
	return(0);
}

// Calculate difference between two points in time
double delay(struct timeval *start, struct timeval *end)
{
    double timesum = (end->tv_sec - start->tv_sec) * 1000;  // seconds to milliseconds
    timesum += (end->tv_usec - start->tv_usec) / 1000;   // microseconds to milliseconds

    return (timesum); // return in seconds
}

void closeProc(int sigNum)
{
    printf("Closing Process...\n");
    exit(0);
}