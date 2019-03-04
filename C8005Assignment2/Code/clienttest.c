#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_TCP_PORT        7000    // Default port
#define BUFLEN            2048      // Buffer length
#define DEFAULT_NUM_SENDS 1
#define DEFAULT_NUM_CLIENTS 0

long delay(struct timeval t1, struct timeval t2);

int main (int argc, char **argv)
{
    int n, bytes_to_read, bytes_to_send, bytes_sent;

    int sd, port, opt;

    struct hostent    *hp;

    struct sockaddr_in server;

    char  *host, *bp, **pptr;

    int msg_len,num_of_sends, num_of_clients;

    char str[16];



    FILE *fp;

    char filename[] = "client_log.csv";

    int num_requests = 0, num_data = 0;

    struct timeval start, end;





    fp = fopen(filename, "w+");

    fprintf(fp, "Process_ID, # of requests, Data sent (B), Response time (ms)...");

    fflush(fp);



    port = SERVER_TCP_PORT;

    msg_len = BUFLEN;

    num_of_sends = DEFAULT_NUM_SENDS;

    num_of_clients = DEFAULT_NUM_CLIENTS;



    while ((opt = getopt(argc, argv, "c:h:l:n:p:")) != -1)

    {

        switch(opt)

        {

            case 'h':

                host =    optarg;    // Host name

            break;

            case 'p':

                port =    atoi(optarg);    // User specified port

            break;

            case 'l':

                msg_len = atoi(optarg);

            break;

            case 'n':

                num_of_sends = atoi(optarg);

            break;

            case 'c':

                num_of_clients = atoi(optarg);

            break;

            default:

                fprintf(stderr, "Usage: %s host [port] [length of string] [# of sends] [# of connections]\n", argv[0]);

                exit(1);

        }

    }



    char sbuf[BUFLEN], rbuf[msg_len], msg[msg_len];

    long response_times[num_of_sends];



    printf("Transmit:\n");



    // get user's text

    fgets (msg, msg_len, stdin);



    pid_t child_pid, pid;



    for(int i = 1; i < num_of_clients; i++)

    {

        if((child_pid = fork()) == 0)

        {

            break;

        }

    }



    pid = getpid();



    // Create the socket

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)

    {

        perror("Cannot create socket");

        exit(1);

    }



    /*

    struct timeval timeout;

    timeout.tv_sec = num_of_clients / 2;

    timeout.tv_usec = 0;



    if (setsockopt (sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,

                sizeof(timeout)) < 0)

    {

        perror("setsockopt failed\n");

    }

    */

    int flags = 1;

    if (setsockopt (sd, SOL_SOCKET, SO_KEEPALIVE, &flags,

                sizeof(flags)) < 0)

    {

        perror("setsockopt failed\n");

    }





    bzero((char *)&server, sizeof(struct sockaddr_in));

    server.sin_family = AF_INET;

    server.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL)

    {

        fprintf(stderr, "Unknown server address\n");

        exit(1);

    }

    bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);



    // Connecting to the server

    if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)

    {

        fprintf(stderr, "Can't connect to server\n");

        perror("connect");

        exit(1);

    }

    printf("Connected:    Server Name: %s\n", hp->h_name);

    pptr = hp->h_addr_list;

    printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));



    for(int i = 0; i < num_of_sends; i++)

    {



        num_requests++;

        printf("Send: %s\n", msg);

        // Transmit data through the socket

        bytes_sent = 0;

        bytes_to_send = msg_len;

        while (bytes_to_send > 0)

        {

            strncpy(sbuf, msg + bytes_sent, BUFLEN);

            send (sd, sbuf, BUFLEN, 0);

            bytes_sent += BUFLEN;

            bytes_to_send -= BUFLEN;

            num_data += BUFLEN;

        }



        gettimeofday(&start, NULL);

        printf("Receive:\n");

        bp = rbuf;

        bytes_to_read = msg_len;



        // client makes repeated calls to recv until no more data is expected to arrive.

        n = 0;

        while ((n = recv (sd, bp, BUFLEN, 0)) < bytes_to_read)

        {

            bp += n;

            bytes_to_read -= n;

        }

        gettimeofday(&end, NULL);

        response_times[i] = delay(start, end);

        printf ("Received: %s\n", rbuf);

    }

    printf ("End of message\n");



    fprintf(fp, "\n%d, %d, %d", pid, num_requests, num_data);

    for(int i = 0; i < num_requests; i++)

    {

        fprintf(fp, ", %ld", response_times[i]);

    }



    fflush(stdout);

    close (sd);



    int status = 0;

    if(child_pid != 0)

        while((child_pid=wait(&status)) > 0);



    return (0);

}



//Compute the delay between t1 and t2 in milliseconds

long delay(struct timeval t1, struct timeval t2)

{

    long d;



    d = (t2.tv_sec - t1.tv_sec) * 1000;

    d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);

    return(d);

}