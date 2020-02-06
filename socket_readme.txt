To compile: On Ubuntu

gcc server.c -o server

gcc client.c -o client

gcc bloomclient.c -o bloomclient

=====================

$ ./client bloomcounty.eng.mu.edu
Socket successfully created..
connected to the server..
Enter the string : Hellothere
From Server : greetings
Enter the string : what's up
From Server : exit
Client Exit...


==========================
$ ./server
Socket successfully created..
Socket successfully binded..
Server listening..
server acccept the client...
From client: Hellothere
	 To client : greetings
From client: what's up
	 To client : exit
Server Exit...

