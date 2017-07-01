#!/bin/python
from socket import *
import sys
import os

#Name : Holly Buteau
#Date : June 4, 2017
#File : Ftclient.py
#Purpose : establish client-side interactions

#Function : Verify
#Purpose : Validates command-line arguments

def verify():

	#all of this will validate the various command line arguments
	if  len(sys.argv) < 5 or len(sys.argv) > 6:
		print "Number of args must be 5 or 6"
		exit(1)
	elif  int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024:
		print "Server port number must be between 1024 and 65535"
		exit(1)
	elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
		print "Invalid Command"
		exit(1)
	if (sys.argv[3] == '-l' and len(sys.argv) != 5):
                print "Invalid number of arguments : Must be 5"
		exit(1)
        if (sys.argv[3]== '-g' and len(sys.argv) != 6):
                print "Invalid number of arguments  : Must be 6"
		exit(1)
	if (sys.argv[3] == '-l' and int(sys.argv[4]) > 65535 or int(sys.argv[4] < 1024)):
		print "Invalid data port number"	 
		exit(1)
	if (sys.argv[3] == '-g' and int(sys.argv[5]) > 65535 or int(sys.argv[5] < 1024)):
                print "Data port number must be between 1024 and 65535"
                exit(1)

#Function : Connection
#Purpose : connects to server
#Returns : socket

def connection():
	#we append ".engr.oregonstate.edu" so the client doesn't have to
	server = sys.argv[1] + ".engr.oregonstate.edu"
	serverPortno = int(sys.argv[2])
	#socket creation
	cliSock = socket(AF_INET, SOCK_STREAM)
	#connect
	cliSock.connect((server, serverPortno))
	return cliSock

#Function : getAddr
#Purpose : gets the IP address of client 

def getAddr():
	#help from: https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
	sockfd = socket(AF_INET, SOCK_DGRAM)
	sockfd.connect(("8.8.8.8", 80))
	return sockfd.getsockname()[0]


#Function : getDir
#Purpose : gets the directory contents from the server

def getDir(data):
	
	fileName = data.recv(100)
	while fileName != "end":
		print fileName
		fileName = data.recv(100)

#Function : getFile
#Purpose : gets the file from the server

def getFile(data):
	#help from : https://docs.python.org/2/tutorial/inputoutput.html
	#and https://learnpythonthehardway.org/book/ex16.html

	newFile = open(sys.argv[4], "w")
	fileBuffer = data.recv(1000)

	#this will indicate the end of the file
	while "__end__" not in fileBuffer:
		newFile.write(fileBuffer)
		fileBuffer = data.recv(1000)

#Function : runProgram
#Purpose : runs the program depending on the client command

def runProgram(cliSock):
	#this was very helpful: https://www.tutorialspoint.com/python/python_networking.htm

	#setting up the client commands
        if sys.argv[3] == "-l":
                print "Directory Request"
                portno = 4
        elif sys.argv[3] == "-g":
                print "File Request for: {}".format(sys.argv[4])
                portno = 5
	fileName = sys.argv[4]
        cliSock.send(sys.argv[portno])
        cliSock.recv(1024)
        if sys.argv[3] == "-l":
                cliSock.send("l")
        elif sys.argv[3] == "-g":
                cliSock.send("g")

        cliSock.recv(1024)

	#send ip address to client
        cliSock.send(getAddr())
        r = cliSock.recv(1024)

	#server doesn't recognize command
        if r == "invalid":
                print "Invalid Command"
                exit(1)

	#all designed to get the file
        if sys.argv[3] == "-g":
		filename, file_ext = os.path.splitext(sys.argv[4])
		cliSock.send(sys.argv[4])
                r = cliSock.recv(1024)

		#if the file doesn't exist:
                if r != "File":
                        print "%Server  says FILE NOT FOUND"
                        return

		#if the file already exists, append _copy to the name of the new file
		if os.path.isfile(sys.argv[4]):
			#print "File already exists, overwriting file"
			#targetfile = open(sys.argv[4] , 'w')
			#targetfile.write(r)
			print "File already exists, appending _copy to new file"
			filename = filename + "_copy"
			sys.argv[4] = filename + file_ext	
	
		print "Transfer Complete"	
	
	#the location of the port number will depend on which command is received
        if sys.argv[3] == "-l":
                portno = 4
        if sys.argv[3] == "-g":
                portno = 5

	#sets up the socket
        dataPortno = int(sys.argv[portno])
        sockfd = socket(AF_INET, SOCK_STREAM)
        sockfd.bind(('', dataPortno))
        sockfd.listen(1)
        data, address = sockfd.accept()


        if portno == 4:
                getDir(data)
        if portno == 5:
                getFile(data)

        data.close()

		

if __name__ == "__main__":
	
	verify()

	cliSock = connection()
	
	runProgram(cliSock)

