Hello, and welcome to Greg and Connor's COMP 8005 Assignemnt #2, A.K.A. The Select & Epoll Server Assignment Based on Scalability and Performance 
(unofficially SESABSP, because acronyms make every project seem important)

Compile:
	Compile select_svr.c using this -> gcc -Wall -o sel_svr select_svr.c threadstack.c -lpthread -g
	Compile epoll_svr.c using this -> gcc -Wall epoll_svr.c -o epoll_svr -lpthread -g
	Compile client2.c using this -> gcc -Wall client2.c -o client2 -lpthread -g

Setup and Config:
	To Setup this program, simply compile the necessary files noted above through the command line. 

How To Use:
	To start this program, run the programs with the "./[filename]" command. As for the client2 file, use the following commands to run the program:
	./client2 [hostip] [number of clients] [f/p] (Optional Flags: [times to send])
	where the f = "file transfer" and p = "send user input paragraph"
	
	Note: The higher the [times to send] number is, the more overlap there will be when sending. So to increase specific load times, this number at a higher caluer should do the trick.

Notes:
	Additional Notes about the Program:
		- The client2 file send feature currently only sends the X.txt files that are sent with the program. Each one contains a basic string to send.
