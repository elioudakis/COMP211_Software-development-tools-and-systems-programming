/* 
 **  remoteClient.c
 **  Author: elioudakis
 **  A.M.: 2018030020
 **  LAB21142447
 **  elioudakis@isc.tuc.gr
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 1024

/* 
 * error -- wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n, UDPport;
	int c; /* used for a character */
	int counter;
    int counter_for_filenames=0;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname, *inputFilename;
    char buf[BUFSIZE];
    FILE* fp, *commandsFile;

    /* check command line arguments */
    if (argc != 5) {
        fprintf(stderr,"Wrong number of arguments!");
        exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    UDPport = atoi(argv[3]);  
    inputFilename= argv[4];


    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
         error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
         fprintf(stderr,"ERROR, no such host as %s\n", hostname);
         exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
        error("ERROR connecting");


    commandsFile=fopen(inputFilename, "r");
    /* fopen() return NULL if last operation was unsuccessful */
    if(commandsFile == NULL)
    {
        /* Unable to open file hence exit */
        printf("Unable to open file.\n");
        printf("Please check whether file exists and you have read privilege.\n");
        exit(EXIT_FAILURE);
    }

	/* We will open the socket as a file stream, in order to write */
		FILE * sock_fp ;
		FILE * pipe_fp ;

    /**
     * getline function is included in stdio.h
     * We use it to read one by one the lines of the file
    **/
    int lines_counter=0;
    char * line = NULL;
    size_t line_len = 0;
    ssize_t read_check;
    while((read_check = getline(&line, &line_len, commandsFile)) != (-1) ){
        /* Read single line from file */
        /* Send the line to the server */
        //printf("%s", line);
        if(lines_counter==10){
            sleep(5);
            lines_counter = 0;
        }
        n = write( sockfd, line, strlen(line));
        lines_counter++;
        if (n < 0)
            error("Error while writing to socket!");   
    }

    /* print the server's reply */
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
        error("ERROR reading from socket");
    
    /* construct the filename */
    char filename[32]="output.";
    char delimiter[1] = ".";
	char portstr[6];
	int portint=portno;

    counter_for_filenames=1; /* it was supposed to change for each command */
    char counter_filename[3];

    sprintf(portstr, "%d", portint);
    strcat(filename, portstr);

    strcat(filename, delimiter);

    sprintf(counter_filename, "%d", counter_for_filenames);
    strcat(filename, counter_filename);

    fp=fopen(filename, "w");
	fputs(buf, fp);
	
    close(sockfd);
    fclose(fp);
    free(line);
    return 0;
}



