#!/usr/bin/env python
from socket import *
import signal
import sys



#Name : Holly Buteau
#Date: May 7, 2017
#Program: Project 1
#File: chatserve.py
#purpose: To creat the chat server
#and connect to the client

#Source: https://docs.python.org/3/howto/sockets.html

#Name: chat
#Input: the server handle and client socket
#output: receives client messages and seneds messages


#source: http://stackoverflow.com/questions/27360218/how-to-close-socket-connection-on-ctrl-c-in-a-python-programme
def signal_handler(signal, frame):
	# close the socket here
	sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

def chat(clientsocket, serverHandle):
	#while the server is connected
	while 1:
		#source: http://stackoverflow.com/questions/21233340/sending-string-via-socket-python
		messages = clientsocket.recv(501)
		sendMessage = ""
		
		if not messages:
			print "Client Disconnected.\n"
			break

		#format and print client message
		print "{}".format(messages)

		#format  message to send 
		while len(sendMessage) == 0 or len(sendMessage) > 500:
			sendMessage = raw_input("{}> ".format(serverHandle))

		#check if  the user wants to disconnect
		if sendMessage == "\quit":
			print "Connection has been closed"
			break

		#send message
		clientsocket.send(sendMessage)




#Name: main
#Input: port number from user
#Output: Calls chat with 
#the client socket and server
#handle


if __name__ == "__main__":
	#hard coded server handle
	serverHandle = "Mouse" 

	#the port number is the second argument passed in
	portno = sys.argv[1]

	#create socket
	serversocket = socket(AF_INET, SOCK_STREAM)

	print "Socket created"
	
	#bind socket, allow the host name to be anything
	try:
		serversocket.bind(('',int(portno)))
	except error as msg:
		print "Bind failed. Error Code : " + str(msg[0]) + ' Message ' + msg[1]
		sys.exit()

	#listen for connection
	serversocket.listen(1)
	print "Now listening"
		
	while 1:
		#call chat with information
		print "Waiting on Connection"
		(clientsocket, address) = serversocket.accept()
		print "Connection Found." 
		chat(clientsocket, serverHandle)
		clientsocket.close()

