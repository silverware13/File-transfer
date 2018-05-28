/* ftserver
 * CS372 Spring 2018
 * -----------------
 * Name: Zachary Thomas
 * Email: thomasza@oregonstate.edu
 * Date: 5/19/2018
 * -----------------
 * Server for file transfer.
 * -----------------------
 * Cited references:
 * 
 * Reviewed my code from my
 * CS344 OTP socket programming
 * assignment.
 * https://github.com/silverware13/OTP/blob/master/otp_enc_d.c
 *
 * Reviewed my code from our last assignment (project1)
 * https://github.com/silverware13/Chat/blob/master/chatclient.c
 *
 * Found a refrence to how to use dirent.h to list the content of directories.
 * https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
 */

#define MAX_CHARS_MESSAGE 5000000
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <dirent.h>

//function prototypes
bool checkArgs(int argc, char *argv[]);
void startup(int controlPort);
void handleRequest(int controlConnection, int controlPort, char *clientName);
void fileTransfer(char *buffer, char type, int dataPort);

int main(int argc, char *argv[])
{
	//make sure the user passed valid arguments
	if(!checkArgs(argc, argv)){
		return 0;
	}

	//set port number	
	int controlPort = strtol(argv[1], NULL, 10);

	//setup the  server and handle all incoming commands
	startup(controlPort);
	
	return 0;
}

/* Function: checkArgs
 * --------------------
 *  Checks to make sure there are two arguments.
 *  Makes sure the portnumber is an unsigned int.
 *
 *  argc: The number of command line arguments.
 *  *argv[]: Array of command line arguments.
 *
 *  returns: Returns true if correct number of 
 *  valid arguments, otherwise returns false.  
 */
bool checkArgs(int argc, char *argv[])
{
	//make sure the user entered correct number of arguments.
	if(argc != 2){
		printf("Useage: %s [port number]\n", argv[0]);
		return false;
	}

	//make sure the user has entered the port number as digits
	if(!isdigit(*argv[1])){
		printf("Please enter port number as an unsigned integer.\n");
		return false;
	}
	
	return true;
}

/* Function: startup
 * --------------------------
 *  Starts up the server and listens
 *  for controlConnections on the given port
 *  number.
 *
 *  controlPort: The port number given on command line.
 */
void startup(int controlPort)
{
	//setup variables
	int socketFD, listen_socket, controlConnection;
	struct sockaddr_in serverAddress, clientAddress;
	struct hostent* server_host_info;
	socklen_t size_client_info;	

	//set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear address struct
	serverAddress.sin_family = AF_INET; //set address family
	serverAddress.sin_port = htons(controlPort); //save port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; //we allow controlConnection from any address

	//set up socket
	listen_socket = socket(AF_INET, SOCK_STREAM, 0); //create the socket

	//start listening with current socket for any incoming controlConnections
	bind(listen_socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); //connect socket to port
	listen(listen_socket, 10); //socket is now listening for a controlConnection

	printf("Server open on %d\n\n", controlPort);	
	
	//we will keep looking for controlConnections until we are terminated
	while(1){
		//accept the next controlConnection
		size_client_info = sizeof(clientAddress); //get the size of the address for the client that will connect
		controlConnection = accept(listen_socket, (struct sockaddr *)&clientAddress, &size_client_info); //accept
		
		//show the connecting client
		char clientName[INET_ADDRSTRLEN];
		memset(clientName, '\0', INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, clientName, sizeof(clientName));
		printf("Connection from %s\n", clientName);

		//react to clients command
		handleRequest(controlConnection, controlPort, clientName);
	
		//close the connection 
		close(controlConnection);
	}
}

/* Function: handleRequest
 * --------------------------
 *  Receive a request from the client
 *  and then send an appropriate response.
 *
 *  controlConnection: The connection number.
 */
void handleRequest(int controlConnection, int controlPort, char *clientName)
{
	//setup variables
	int bufLen, charsRead;
	char portBuffer[100];
	char serverName[1000];
	char fileName[1000];
	int bufSum = 0; //the number of chars we have writen to our buffer
	char *buffer;
	buffer = (char *)malloc(MAX_CHARS_MESSAGE * sizeof(char));
	memset(buffer, '\0', MAX_CHARS_MESSAGE);
	
	//read message from the client
	do{
		charsRead = recv(controlConnection, &buffer[bufSum], 100, 0); //read from socket
		bufSum += charsRead;
		bufLen = strlen(buffer);
		if(charsRead < 0){
			fprintf(stderr, "Error reading from socket.\n");
			exit(2); 
		}
	} while(buffer[bufLen - 1] != '\n');

	//get the data port
	int i = 0;
	do{
		portBuffer[i] = buffer[i];
		bufLen = strlen(portBuffer);
		i++;
	} while(portBuffer[bufLen - 2] != '-');

	//get the type of request
	char type = portBuffer[bufLen - 1];
	
	//remove the - from the portBuffer
	portBuffer[bufLen - 2] = '\0';
	int dataPort = strtol(portBuffer, NULL, 10);
	
	//get the server name
	int ii = 0;
	do{
		serverName[ii] = buffer[i];
		bufLen = strlen(serverName);
		i++;
		ii++;
	} while(serverName[bufLen - 1] != '@');
	serverName[bufLen - 1] = '\0';
	
	//get requested file name if not list mode
	ii = 0;
	if(type == 'g'){
		do{
			fileName[ii] = buffer[i];
			bufLen = strlen(fileName);
			i++;
			ii++;
		} while(fileName[bufLen - 1] != '\n');
		fileName[bufLen - 1] = '\0';
	}

	//print to server how request is being handled
	if(type == 'l'){
		printf("List directory requested on port %d\n", dataPort);	
		printf("Sending directory contents to %s:%d\n\n", clientName, dataPort);	
		memset(buffer, '\0', MAX_CHARS_MESSAGE);
		sprintf(buffer, "Receiving directory structure from %s:%d", serverName, dataPort);
	} else {
		printf("File \"%s\" requested on port %d\n", fileName, dataPort);
		//confirm that the file exists
		if( access( fileName, F_OK ) != -1 ) {
			printf("Sending \"%s'\" to %s:%d\n\n", fileName, clientName, dataPort);
			memset(buffer, '\0', MAX_CHARS_MESSAGE);
			sprintf(buffer, "Receiving \"%s\" from %s:%d\n", fileName, serverName, dataPort);
		} else {
			type = 'n';
			printf("File not found. Sending error message to %s:%d\n\n", clientName, dataPort);
			memset(buffer, '\0', MAX_CHARS_MESSAGE);
			sprintf(buffer, "%s:%d says FILE NOT FOUND", serverName, controlPort);
		}	
	}
	
	//send message to client
	int charsWritten = 0;
	do{
		charsWritten += send(controlConnection, buffer, strlen(buffer), 0); //write to socket
		if(charsWritten < 0){
			fprintf(stderr, "Error writing to socket.\n");
			exit(2); 
		}
	} while(charsWritten < strlen(buffer));
	
	//transfer the file
	fileTransfer(buffer, type, dataPort);
}

/* Function: fileTransfer
 * --------------------------
 */
void fileTransfer(char *buffer, char type, int dataPort)
{
	//setup variables
	int socketFD, listen_socket, dataConnection;
	struct sockaddr_in serverAddress, clientAddress;
	struct hostent* server_host_info;
	socklen_t size_client_info;	

	//set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear address struct
	serverAddress.sin_family = AF_INET; //set address family
	serverAddress.sin_port = htons(dataPort); //save port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; //we allow connection from any address

	//set up socket
	listen_socket = socket(AF_INET, SOCK_STREAM, 0); //create the socket
	if(listen_socket < 0){
		fprintf(stderr, "Error on data socket\n");
		return;
	}	

	//start listening with current socket for any incoming controlConnections
	bind(listen_socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); //connect socket to port
	listen(listen_socket, 10); //socket is now listening for a dataConnection
	
	printf("LISTENING FOR DATA CONNECTION\n");
	//accept the next connection
	size_client_info = sizeof(clientAddress); //get the size of the address for the client that will connect
	dataConnection = accept(listen_socket, (struct sockaddr *)&clientAddress, &size_client_info); //accept
	if(dataConnection < 0){
		fprintf(stderr, "Error on data accept\n");
		return;	
	}
	printf("ABOUT TO SEND FILEEEE\n");
	
	//send file
	if(type == 'l'){
		char *buffer_ptr = buffer;
		struct dirent *dir_entry;
		DIR *dir = opendir(".");
		while((dir_entry = readdir(dir)) != NULL) {
			buffer_ptr += sprintf(buffer_ptr, "%s ", dir_entry->d_name);
		}
	} else if (type == 'g'){
		sprintf(buffer, "A FILE");
	} else {
		//no valid file to send
		close(dataConnection);
		return;
	}

	//send file to client
	int charsWritten = 0;
	do{
		charsWritten += send(dataConnection, buffer, strlen(buffer), 0); //write to socket
		if(charsWritten < 0){
			fprintf(stderr, "Error writing to socket.\n");
			exit(2); 
		}
	} while(charsWritten < strlen(buffer));
	close(dataConnection);
}
