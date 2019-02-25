/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		epoll_svr.c -   A simple echo server using TCP
--
--	PROGRAM:			epoll.exe
--
--	FUNCTIONS:		Berkeley Socket API
--
--	DATE:				February 25, 2019
--
--	REVISIONS:			(Date and Description)
--
--	DESIGNERS:			Greg Little and Connor Phalen
--
--	PROGRAMMER:		Greg Little And Connor Phalen
--
--	NOTES:
--	The program will accept TCP connections from multiple client machines.
-- 	The program will read data from each client socket and simply echo it back.
--  ulimit -n 100000
---------------------------------------------------------------------------------------*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define TRUE 1
#define EPOLL_QUEUE_LEN	1024
#define BUFLEN		1024
#define SERVER_PORT	8080
#define THREAD_INIT 1000000
//Globals
int fd_server;
// need to pass epoll_fd, events, remote_addr,

struct targs{
	struct epoll_event event;
};

struct targs2{
	int epoll_fd;
	struct epoll_event events[EPOLL_QUEUE_LEN];
	struct epoll_event event;
	int i;
	int count;
};


// Function prototypes
static void SystemFatal (const char* message);
void* threadWorker(void *arguments);

void* accepter(void *arguments);

static int ClearSocket (int fd);
void close_fd (int);

int main (int argc, char* argv[])
{
	int i, arg, count;
	int num_fds, epoll_fd;
	static struct epoll_event events[EPOLL_QUEUE_LEN], event;
	int port = SERVER_PORT;
	struct sockaddr_in addr;
	struct sigaction act;
	pthread_t other[THREAD_INIT];//should be linked list

	// set up the signal handler to close the server socket when CTRL-c is received
        act.sa_handler = close_fd;
        act.sa_flags = 0;
        if ((sigemptyset (&act.sa_mask) == -1 || sigaction (SIGINT, &act, NULL) == -1))
        {
                perror ("Failed to set SIGINT handler");
                exit (EXIT_FAILURE);
        }

	// Create the listening socket
	fd_server = socket (AF_INET, SOCK_STREAM, 0);
    	if (fd_server == -1)
		SystemFatal("socket");

    	// set SO_REUSEADDR so port can be resused imemediately after exit, i.e., after CTRL-c
    	arg = 1;
    	if (setsockopt (fd_server, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
		SystemFatal("setsockopt");

    	// Make the server listening socket non-blocking
    	if (fcntl (fd_server, F_SETFL, O_NONBLOCK | fcntl (fd_server, F_GETFL, 0)) == -1)
		SystemFatal("fcntl");

    	// Bind to the specified listening port
    	memset (&addr, 0, sizeof (struct sockaddr_in));
    	addr.sin_family = AF_INET;
    	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    	addr.sin_port = htons(port);
    	if (bind (fd_server, (struct sockaddr*) &addr, sizeof(addr)) == -1)
		SystemFatal("bind");

    	// Listen for fd_news; SOMAXCONN is 128 by default
    	if (listen (fd_server, SOMAXCONN) == -1)
		SystemFatal("listen");

    	// Create the epoll file descriptor
    	epoll_fd = epoll_create(EPOLL_QUEUE_LEN);
    	if (epoll_fd == -1)
		SystemFatal("epoll_create");

    	// Add the server socket to the epoll event loop
    	event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    	event.data.fd = fd_server;
    	if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, fd_server, &event) == -1)
		SystemFatal("epoll_ctl");

	// Execute the epoll event loop
		count = 0;
    	while (TRUE)
	{
		//struct epoll_event events[MAX_EVENTS];
		num_fds = epoll_wait (epoll_fd, events, EPOLL_QUEUE_LEN, -1);

		if (num_fds < 0)
			SystemFatal ("Error in epoll_wait!");

//here
		for (i = 0; i < num_fds; i++)
		{
	    		// Case 1: Error condition
	    		if (events[i].events & (EPOLLHUP | EPOLLERR))
			{
				fputs("\nepoll: EPOLLERR\n", stderr);
				close(events[i].data.fd);
				continue;
	    		}
	    		assert (events[i].events & EPOLLIN);

	    		// Case 2: Server is receiving a connection request
	    		if (events[i].data.fd == fd_server)
			{
				printf("counter: %d\n", count);
				count++;
				//pass in epoll_fd events[] event i
				struct targs2 args2 = (struct targs2){epoll_fd, *events, event, i, count};
				if(pthread_create(&other[count], NULL, &accepter, (void *)&args2) != 0)
				{
					printf("Error Occured with Thread Creation");
					return -1;
				}
		//		printf("i:%d fd: %d \n",i,events[i].data.fd);
			}
	}
	}//end of while

	//printf("counter: %d\n", count);


	close(fd_server);
	exit (EXIT_SUCCESS);
}


void* accepter(void *arguments){
	struct targs2 *args2 = arguments;
	int i = args2->i;
	int count = args2->count;
	int epoll_fd = args2->epoll_fd;
	struct epoll_event event = args2->event;
	struct epoll_event events[EPOLL_QUEUE_LEN];
	events[i] = args2->events[i];

	pthread_t threads[THREAD_INIT];
	struct sockaddr_in remote_addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int fd_new = accept (fd_server, (struct sockaddr*) &remote_addr, &addr_size);

	if (fd_new == -1)
	{
				if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			perror("accept");
				}
				//continue;
	}

	// Make the fd_new non-blocking
	if (fcntl (fd_new, F_SETFL, O_NONBLOCK | fcntl(fd_new, F_GETFL, 0)) == -1)
		SystemFatal("fcntl");

	// Add the new socket descriptor to the epoll loop
	event.data.fd = fd_new;
	if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, fd_new, &event) == -1)
		SystemFatal ("epoll_ctl");

		// set up arguments for threads
		//eventWorker = event;
		if(ClearSocket(event.data.fd)==0){
			//printf("nothing here\n");
		}
/*		struct targs args = (struct targs){events[i]};
		if(pthread_create(&threads[count], NULL, &threadWorker, (void *)&args) != 0)
		{
			printf("Error Occured with Thread Creation");
			return -1;
		}*/

	printf("%s ", inet_ntoa(remote_addr.sin_addr));
			close(event.data.fd);
		pthread_exit(NULL);
}

void* threadWorker(void *arguments){
	struct targs *args = arguments;
	struct epoll_event event = args->event;
	printf("%ld\n", pthread_self());
	while(TRUE){
		//printf("eventData %d\n",event.data.fd);
		if(ClearSocket(event.data.fd)==0){
			//printf("nothing here\n");
			break;
		}

		//close(event.data.fd);
		//need a way around closing event.data.fd
	}
	//printf("threadDead\n");
		close(event.data.fd);
	pthread_exit(NULL);
	return 0;
}



static int ClearSocket (int fd)
{

	int n, bytes_to_read=0;
	char	*bp, buf[BUFLEN];
	while (TRUE)
	{
		n=1;
		bytes_to_read = 0;
		bzero(buf,BUFLEN);
		bp = buf;
		bytes_to_read = BUFLEN;
		while ((n = recv (fd, bp, bytes_to_read, 0)) < bytes_to_read)
		{
			if(n==0){
				return 0;
			}else if(n!=-1){
				bp += n;
				bytes_to_read -= n;
			}
		}
		//printf("N:%d, to_read: %d\n", n,bytes_to_read);
		//printf ("sending:%s\n", buf);
		fflush(stdout);
		send (fd, buf, BUFLEN, 0);
		//close (fd);
		return 1;
	}
	close(fd);
	return(0);

}

// Prints the error stored in errno and aborts the program.
static void SystemFatal(const char* message)
{
    perror (message);
    exit (EXIT_FAILURE);
}

// close fd
void close_fd (int signo)
{
        close(fd_server);
	exit (EXIT_SUCCESS);
}
