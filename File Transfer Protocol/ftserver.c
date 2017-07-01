/***************************************
 * Name : Holly Buteau
 * Date : June 4, 2017
 * File : Ftserver.c
 * Description : handles server side
 * responsibilities 
***************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

struct addrinfo * getinfo(char *port);
void start(int r);
int newSocket(struct addrinfo *res);
void bindSocks(int sockfd, struct addrinfo *res);
void listenSocks(int sockfd);
void connectSocks(int sockfd, struct addrinfo *res);
void connecting(int sockfd);
char ** createDir(int length);
int getDir(char ** directory);
void sendDir(char *addr, char *portno, char **directory, int fileCount);
void deleteContents(char ** files, int length);
int verifyFile(char ** file, int fileCount, char *fileName);
void fileSend(char *addr, char *portno, char *fileName);
struct addrinfo * getinfoWithIp(char *addr, char *portno);

/*************************************
 * Function : getinfo
 * Arguments : the port number
 * Returns : address info
 * Purpose : gets address info for 
 * port number
************************************/

struct addrinfo * getinfo(char *port)
{
	//help from: http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
	int stat;
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((stat = getaddrinfo(NULL, port, &hints, &res)) != 0)
	{
		printf("Error: Port number isn't valid\n",
		gai_strerror(stat));
		exit(1);
	}

	return res;
} 

/*****************************************
 * Function : getinfoWithIp
 * Arguments : the port number and address
 * Returns : address info
 * Purpose : gets address info for
 * port number and address
 ****************************************/


struct addrinfo * getinfoWithIp(char *addr, char *portno)
{
	//help from: http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
	int stat;
	struct addrinfo hints;
	struct addrinfo *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((stat = getaddrinfo(addr, portno, &hints, &res)) != 0)
        {
                printf("Error: Port number isn't valid\n",
                gai_strerror(stat));
                exit(1);
        }

        return res;
	
}	

/*************************************
 * Function : start
 * Arguments : socket 
 * Returns : nothing
 * Purpose : reads client command 
 * and handles it 
************************************/


void start(int r)
{
        //help from : https://en.wikibooks.org/wiki/C_Programming/Networking_in_UNIX
        
        //validating the request
        const char *validReq = "valid";
        const char *invalidReq = "invalid";
        char portno[100];
        memset(portno, 0, sizeof(portno));
        recv(r, portno, sizeof(portno) - 1, 0);
        send(r, validReq, strlen(validReq), 0);
        char cliCommand[100];
        memset(cliCommand, 0, sizeof(cliCommand));
        recv(r, cliCommand, sizeof(cliCommand) - 1, 0);
        send(r, validReq, strlen(validReq), 0);

        char addr[100];
        memset(addr, 0, sizeof(addr));
        recv(r, addr, sizeof(addr) - 1, 0);
        printf ("Connection from client\n");

	//this will handle the request for directory listing
        if(strcmp(cliCommand, "l") == 0)
        {
                send(r, validReq, strlen(validReq), 0);
                printf("List directory requested on port %s\n", portno);
                printf("Sending directory contents to %s \n", portno);
                char ** directory = createDir(100);
		int fileCount = getDir(directory);
                sendDir(addr, portno, directory, fileCount);
                deleteContents(directory, 100);
        }

	
	//this will handle the request for file 
       if(strcmp(cliCommand, "g") == 0)
       {
               send(r, validReq, strlen(validReq), 0);
               char fileName[100];
               memset(fileName, 0, sizeof(fileName));
               recv(r, fileName, sizeof(fileName) - 1, 0);
               printf("File %s requested on port %s\n", fileName, portno);
               char ** file = createDir(100);
               int fileCount = getDir(file);
               int trueFile = verifyFile(file, fileCount, fileName);
		
		//if the file exists in the directory
               if(trueFile)
               {
                      printf("Sending %s to client on: %s\n", fileName, portno);
                       const char *fileTrue = "File";
                       send(r, fileTrue, strlen(fileTrue),0);
			char newFile[100];
                       memset(newFile, 0, sizeof(newFile));
                       strcpy(newFile, "./");
                       char *endFile = newFile + strlen(newFile);
                       endFile += sprintf(endFile, "%s", fileName);
			fileSend(addr, portno, newFile);
               }

		//if the files does not exist, send error
               else
               {
                       printf("File not found. Sending error message to client on: %s\n", portno);
                       const char * noFile = "File not here.";
                       send(r, noFile, 100, 0);
               }
		
             deleteContents(file, 100);
      }

      else
      {
              send(r, invalidReq, strlen(invalidReq), 0);
              printf("Client sent invalid command\n");
      }

       printf("Waiting for conneciton\n");
}

/*************************************
 * Function : newSocket
 * Arguments : address info
 * Returns : the return of the call
 * to socket
 * Purpose : creates a socket
*************************************/

int newSocket(struct addrinfo *res)
{
	//help from : http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
	int sockfd;
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	{
		printf("Error with creating socket\n");
		exit(1);
	}
	
	return sockfd;
}

/*************************************
 * Function : bindSocks
 * Arguments : socket and address info
 * Returns : nothing
 * Purpose : binds socket to port
**************************************/


void bindSocks(int sockfd, struct addrinfo *res)
{
	//help from: http://man7.org/linux/man-pages/man3/getaddrinfo.3.html	
	
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
	{
		close(sockfd);
		printf("Error with binding\n");
		exit(1);
	}
}

/*************************************
 * Function : listenSocks
 * Arguments : socket
 * Returns : nothing
 * Purpose : listens on port
************************************/


void listenSocks(int sockfd)
{
	//this came mostly from my operating systems project OTP
	if (listen(sockfd, 5) == -1)
	{
		close(sockfd);
		printf("Error on listen");
		exit(1);
	}
}

/***************************************
 * Function : getinfo
 * Arguments : socket and address
 * Returns : nothing
 * Purpose : connects socket to address
***************************************/


void connectSocks(int sockfd, struct addrinfo *res)
{
	//help from: http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
	int stat;
	if ((stat = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
	{
		printf("Error on connect.\n");
		exit(1);
	}
}

/*************************************
 * Function : connecting
 * Arguments : socket
 * Returns : nothing
 * Purpose : waits for the connection
 * then calls start when received
************************************/

void connecting(int sockfd)
{
	//help from: http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
	//and http://beej.us/guide/bgnet/output/html/multipage/acceptman.html
	struct sockaddr_storage clientAddr;
	
	socklen_t clientAddrLen;

	int r;

	while(1)
	{
		
		clientAddrLen = sizeof(clientAddr);
		r = accept(sockfd, (struct sockaddr *) &clientAddr, &clientAddrLen);
		if (r == -1)
			continue;

		start(r);
		close(r);
	}
}

/*************************************
 * Function : createDir
 * Arguments : the number of files
 * Returns : an array
 * Purpose : creates an array for 
 * files
************************************/


char ** createDir(int length)
{

	char ** newArray = (char**)malloc(length*sizeof(char *));
	int x;
	for (x = 0; x < length; x++)
	{
		newArray[x] = (char*)malloc(100*sizeof(char));
		memset(newArray[x], 0, sizeof(newArray[x]));
	}

	return newArray;
}

/*************************************
 * Function : getDir
 * Arguments : array
 * Returns : number of files in the 
 * directory 
 * Purpose : counts the number of 
 * files in the directory
 ************************************/


int getDir(char ** directory)
{
	//help from : http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
	DIR *d;
	struct dirent * dir;
	d = opendir(".");
	int x = 0;

	if(d)
	{
		while((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG)
			{
				strcpy(directory[x], dir->d_name);
				x++;
			}
		}

		closedir(d);
	}
	
	return x;
} 

/*************************************
 * Function : sendDir
 * Arguments : client address, port
 * number, files in the directory,
 * file count
 * Returns : nothing
 * Purpose : sends the files in the 
 * directory to the client
 * ************************************/


void sendDir(char *addr, char *portno, char **directory, int fileCount)
{
	//some help from here: https://stackoverflow.com/questions/11952898/c-send-and-receive-file
	sleep(2);
	struct addrinfo *res = getinfoWithIp(addr, portno);
	int clientSock = newSocket(res);
	connectSocks(clientSock, res);
	int x;
	for(x = 0; x < fileCount; x++)
		send(clientSock, directory[x], 100, 0);

	const char *finished = "end";
	send(clientSock, finished, strlen(finished), 0);
	close(clientSock);
	freeaddrinfo(res);
}

/*************************************
 * Function : deleteContents
 * Arguments : array of files, number
 * of files in the array
 * Returns : nothing
 * Purpose : deletes the array
************************************/


void deleteContents(char ** files, int length)
{
	int x;
	for (x = 0; x < length; x++)
		free(files[x]);

	free(files);
}

/*************************************
 * Function : verifyFile
 * Arguments : array of files, file
 * count, name of the file to 
 * verify
 * Returns : boolean of whether the
 * file exists
 * Purpose : determines if the file
 * exists
************************************/


int verifyFile(char ** file, int fileCount, char *fileName)
{
	int verify = 0;
	int x;
	for(x = 0; x < fileCount; x++)
	{
		if(strcmp(file[x], fileName) == 0)
		{
			verify = 1;
		}
	}

	return verify;
}

/*************************************
 * Function : fileSend
 * Arguments : address of client,
 * port number, name of file
 * Returns : nothing
 * Purpose : sends files to client
************************************/


void fileSend(char *addr, char *portno, char *fileName)
{
	//help from: https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
	sleep(2);

	struct addrinfo *res = getinfoWithIp(addr, portno);
	int data = newSocket(res);
	connectSocks(data,res);
	char buff[1000];
	memset(buff, 0, sizeof(buff));
	int file = open(fileName, O_RDONLY);
	while(1)
	{
		int count = read(file, buff, sizeof(buff) - 1);
		if (count == 0)
			break;

		if (count < 0)
		{
			printf("Error on file read");
			return;
		}

		void *x = buff;

		while (count > 0)
		{
			int count2 = send(data, x, sizeof(buff), 0);
			if (count2 < 0)
			{
				printf("Error on socket\n");
				return;
			}

			count -= count2;
			

		}

		memset(buff, 0, sizeof(buff));
	}

	memset(buff, 0, sizeof(buff));
	strcpy(buff, "__end__");
	send(data, buff, sizeof(buff), 0);
	close(data);
	freeaddrinfo(res);
}

/*************************************
 * Function : main
 * Arguments : command line arguments
 * Returns : 0 to indicate exit
 * Purpose : starts the program
************************************/

		
	
int main(int argc, char *argv[])
{
	//help from: http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
	if(argc != 2)
	{
		printf ("Not enough arguments\n");
		exit(1);
	}

	//verify the command line arguments
	int port = atoi(argv[1]);
	if (port > 65535 || port < 1024)
	{
		printf("Invalid Port Number\n");
		exit(1);
	}

		
	printf ("Server open on %s\n", argv[1]);

	struct addrinfo *res = getinfo(argv[1]);
	
	int sockfd = newSocket(res);
	
	bindSocks(sockfd, res);
	listenSocks(sockfd);
	connecting(sockfd);
	freeaddrinfo(res);
}
