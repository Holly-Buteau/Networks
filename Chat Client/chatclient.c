/*********************************
 * Name: Holly Buteau
 * Title: Projest 1
 * File: chatclient.c
 * Date: May 7. 2017
 * Purpose: set up a chat client
 * and connect to a server
 *********************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

/*The majority of this code came from my 
Operating Systems Program 4 - OTP assignment
I hit snags in some places and used http://www.linuxhowtos.org/C_C++/socket.htm
for reference. I noted the source in those places*/

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int sockfd;
	int portno;
	int sendMsg;
	int getMsg;
	int quit = 0;
	char handle[11];
	char messageSent[500];
	char messageReceived[500];
	memset(messageSent,0,sizeof(messageSent));
	memset(messageReceived,0,sizeof(messageReceived));
	char msgHandle[510];
	char *pos;

	//all of this goes into creating the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	portno = atoi(argv[2]);
	server = gethostbyname(argv[1]);
	//simple error checking
	if (sockfd < 0) 
	{
		printf("ERROR opening socket");
		exit(1);
	}

	printf("made socket\n");

	//all of this goes into connecting to the server
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	//source: http://www.linuxhowtos.org/C_C++/socket.htm
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	//dertermining if I can connect to the server. This error checking was
	//invaluable to me
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		printf("ERROR connecting to port %d\n", portno);
		exit(2);
	}
	
	printf("Connected to server.\n");
	bzero(handle, 11);
	//getting handle from user
        printf("What is your Handle: ");
        fgets(handle, 10, stdin);
	if ((pos=strchr(handle, '\n')) != NULL)
   		 *pos = '\0';

        /*quit is a bool to determine when to stop prompting
        the user for messgaes. We start with it being 0. 
        When the user types \quit, then we set quit to 1
        and the program stops asking for messages*/

	while(quit == 0)
	{
		printf("%s> ", handle);
		bzero(messageSent, 500);
		fgets(messageSent,500,stdin);

		//this combines the user's message with their handle
		//to send to the server
		
		sprintf(msgHandle,"%s > %s", handle, messageSent); 

		//if user is ready to quit
		if(strcmp(messageSent, "\\quit\n") == 0)
		{
			quit = 1;	
			printf("You are disconnecting.\n");
			break;
			
		}
		
		//send message to server
		sendMsg = send(sockfd, msgHandle, strlen(msgHandle), 0);
		bzero(messageReceived, 500);

		if(sendMsg == -1)
		{
			fprintf(stderr, "Error when sending data to host\n");
			exit(1);
		}
	
		//receieving message from server
		getMsg = recv(sockfd, messageReceived, 500, 0);
		
		if(getMsg == 0)
		{
			printf("You were disconnected from the server\n");
			quit = 1;
		}

		else
		{	//hard coding the server's handle as Mouse 
			//and printing the message received
			printf("Mouse> %s\n", messageReceived);
		}

	}

	close(sockfd);
	return 0;
}

