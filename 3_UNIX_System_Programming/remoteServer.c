/* 
 **  remoteServer.c
 **  Author: elioudakis
 **  elioudakis@isc.tuc.gr
 */

	#include <stdio.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <string.h>
	#include <netdb.h>
	#include <sys/types.h> 
	#include <sys/socket.h>
	#include <sys/wait.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <signal.h>
	#include <errno.h>


	#define BUFSIZE 1024

	/*
	* error - wrapper for perror
	*/
	void error(char *msg) {
	perror(msg);
	exit(1);
	}

	int main(int argc, char **argv) {
		int parentfd; /* parent socket */
		int childfd; /* child socket */
		int portno; /* port to listen on */
		int clientlen; /* byte size of client's address */
		struct sockaddr_in serveraddr; /* server's addr */
		struct sockaddr_in clientaddr; /* client addr */
		char buf[BUFSIZE]; /* message buffer */
		int optval; /* flag value for setsockopt */
		int n; /* message byte size */
		int connectcnt; /* number of connection requests */
		int notdone;
		fd_set readfds;
		
		/* for the fork loop */
		int i=0; /* counter for the fork loop */
		int numChildren = atoi(argv[2]);
		int status = 0;

		/* 
		* check command line arguments 
		*/
		if (argc != 3) {
			fprintf(stderr,"Wrong number of arguments!");
			exit(1);
		}
		portno = atoi(argv[1]);

		signal(SIGPIPE, SIG_IGN);
		
		/*
		*  creating the children 
		*/
		//printf("I am the parent.  My pid is:%d  my ppid is %d\n", getpid(), getppid() );
		for(i=0;i<numChildren;i++){
			pid_t pid=fork();

			if (pid==0) /* only execute this if child */
			{
				//printf("I am a child.  My pid is:%d  my ppid is %d\n",  getpid(), getppid() );
				exit(0);
			}
			wait(&status);  /* only the parent waits */
		}

		/* 
		* socket: create the parent socket 
		*/
		parentfd = socket(AF_INET, SOCK_STREAM, 0);
		if (parentfd < 0) 
			error("ERROR opening socket");

		/* setsockopt: Handy debugging trick that lets 
		* us rerun the server immediately after we kill it; 
		* otherwise we have to wait about 20 secs. 
		* We use it to avoid  "ERROR on binding: Address already in use" error. 
		*/
		optval = 1;
		setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

		/*
		* build the server's Internet address
		*/
		bzero((char *) &serveraddr, sizeof(serveraddr));

		/* this is an Internet address */
		serveraddr.sin_family = AF_INET;

		/* let the system figure out our IP address */
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

		/* this is the port we will listen on */
		serveraddr.sin_port = htons((unsigned short)portno);

		/* 
		* bind: associate the parent socket with a port 
		*/
		if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
			error("ERROR on binding");

		/* 
		* listen: make this socket ready to accept connection requests 
		*/
		if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
			error("ERROR on listen");


		clientlen = sizeof(clientaddr);
		notdone = 1;

		/* 
		* main loop: wait for connection request.
		*
		*/
		while (notdone) {

			/* 
			* select: Has a connection request arrived?
			*/
			FD_ZERO(&readfds);          /* initialize the fd set */
			FD_SET(parentfd, &readfds); /* add socket fd */
			FD_SET(0, &readfds);        /* add stdin fd (0) */
			if (select(parentfd+1, &readfds, 0, 0, 0) < 0) {
				error("ERROR in select");
			}   

			/* if a connection request has arrived, process it */
			if (FD_ISSET(parentfd, &readfds)) {
			/* 
			* accept: wait for a connection request 
			*/
			childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
			if (childfd < 0) 
				error("ERROR on accept");
			
			/* 
			* read: read input string from the client
			*/
			bzero(buf, BUFSIZE);
			n = read(childfd, buf, BUFSIZE);
			if (n < 0) 
				error("ERROR reading from socket");

			
			/* 
			*  answer to the client
			*/

			int c;
			FILE * sock_fp ;
			FILE * pipe_fp ;
			/* open childfd as a file pointer */
			if (( sock_fp = fdopen ( childfd , "r+" ) ) == NULL )
				error( " fdopen " ) ;

			/*
			* execute the commands one by one
			*/
			char character;
			int for_counter;
			char command[100];
			int command_counter=0;

			char end_str[4]="end";
			char stop_str[12]="timeToStop";
			char com1[4]="ls";
			char com2[4]="cat";
			char com3[4]="cut";
			char com4[8]="grep";
			char com5[4]="tr";

			char kill_com[24]="pkill -P ";
			char temp[8];
			sprintf(temp, "%d", getpid());
			
			strcat(kill_com, temp);

			for(for_counter=0; for_counter<strlen(buf); for_counter++){
				
				character = buf[for_counter];
				if(character != ';'  && character != '\n'){
					command[command_counter] = character;
					command_counter++;
				}
				else
				{
					command_counter = 0;
					
					if(strncmp(command, end_str, 3) == 0 )
						exit(0);
					else if(strncmp(command, stop_str, 10) == 0){
						popen(kill_com, "r" );
						exit(0);
					}
					else if((!(strncmp(command, com1, 2))) && (!(strncmp(command, com2, 3))) && (!(strncmp(command, com3, 3))) && (!(strncmp(command, com4, 4))) && (!(strncmp(command, com5, 2))) ){
						memset(command, 0, 100);  // Setting all the bytes to 0.
						//continue;
					}
						
					if (( pipe_fp = popen ( command , "r" ) ) == NULL )
						error ( " popen " ) ;
						
					while ( ( c = getc ( pipe_fp ) ) != EOF )
						putc (c , sock_fp ) ; 
					pclose ( pipe_fp ) ; 
					memset(command, 0, 100);  // Setting all the bytes to 0.
				}
			}

			fclose ( sock_fp ) ;
			close(childfd);
			}
		}

		//Every process which is terminating, is writing to the stderr
		fprintf(stderr, "%d", getpid());

		close(parentfd);
		exit(0);
	}
