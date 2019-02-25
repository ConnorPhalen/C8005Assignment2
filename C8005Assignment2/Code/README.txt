Hello, and welcome to Greg and Connor's COMP 8005 Assignemnt #2, A.K.A. The Select & Epoll Server Assignment Based on Scalability and Performance 
(unofficially SESABSP, because acronyms make every project seem important)

Compile:
	Compile select_svr.c using this -> gcc -Wall -o sel_svr select_svr.c threadstack.c -lpthread -g
	Compile epoll_svr.c using this -> gcc -Wall epoll_svr.c -o epoll_svr -lpthread -g
	Compile client2.c using this -> gcc -Wall client2.c -o client2 -lpthread -g

Setup and Config:
	To Setup this program, simply compile the necessary files noted above through the command line. 

	It also might be a good idea to type "ulimit -n 100000" into the command line, specifying to the system to raise the user-space limit to 10000

How To Use:
	To start this program, run the programs woth the "./[filename]" command. As for the client2 file, use the following commands to modify the output:
	./client2 [ip address] [num of clients] [f or p] 
	where the f = "file transfer" and p = "send user input paragraph"

Notes:
	Additional Notes about the Program:
		- The client2 program runs into some issues when running it striaght up through the command line, although running it
 through GDB makes it run much better. GDB does slow the program down a lot, so it isn't ideal, but atleast it shows a bit of scalability metric.
		- The threadstack.c program isn't used in the program, as the select_svr cleanup section is fully completed. It isn't meeded,
 but it is kept and added to the compile line in case of future updates/legacy editions.
