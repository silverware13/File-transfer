/* ftserver
 * CS372 Spring 2018
 * -----------------
 * Name: Zachary Thomas
 * Email: thomasza@oregonstate.edu
 * Date: 5/19/2018
 * -----------------
 * Starts up a server that listens
 * for control connections on a control port.
 * After receiving a connection will communicate with
 * client and take request. If a file is requested
 * that file is sent over a new connection on the data port.
 * after request has been handled the server closes the 
 * control and data port connections and waits for a new
 * request.
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

#define MAX_CHARS_MESSAGE 900000000
#define MAX_CHARS_FILE_NAME 10000
#define MAX_FIELD_SIZE 100
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
void fileTransfer(char *buffer, char type, int dataPort, char *fileName);

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
 *  for control connections on the given port
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

		//free memory
		free(fileName);		
		free(buffer);		
	
		//close the connection
		close(controlConnection);
	}
}

/* Function: handleRequest
 * --------------------------
 *  Receive a request from the client,
 *  interprets the request, and then send an 
 *  appropriate response.
 *
 *  controlConnection: The connection number.
 *  controlPort: The connection port.
 *  clientName: A string of the clients address.
 */
void handleRequest(int controlConnection, int controlPort, char *clientName)
{
	//setup variables
	int bufLen, charsRead;
	char portBuffer[MAX_FIELD_SIZE];
	char serverName[MAX_FIELD_SIZE];
	int bufSum = 0; //the number of chars we have writen to our buffer
	char *fileName;
	fileName = (char *)malloc(MAX_FIELD_SIZE * sizeof(char));
	memset(fileName, '\0', MAX_FIELD_SIZE);
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
	printf("%s\n", buffer);

	//get the data port
	int i = 0;
	do{
		portBuffer[i] = buffer[i];
		bufLen = strlen(portBuffer);
		i++;
		if(i == MAX_FIELD_SIZE) break;
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
		if(ii == MAX_FIELD_SIZE) break;
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
			if(ii == MAX_FIELD_SIZE) break;
		} while(fileName[bufLen - 1] != '\n');
		fileName[bufLen - 1] = '\0';
	}

	//calculate how request should be handled
	if(type == 'l'){
		printf("List directory requested on port %d\n", dataPort);	
		printf("Sending directory contents to %s:%d\n\n", clientName, dataPort);	
		memset(buffer, '\0', MAX_CHARS_MESSAGE);
		sprintf(buffer, "Receiving directory structure from %s:%d", serverName, dataPort);
	} else {
		printf("File \"%s\" requested on port %d\n", fileName, dataPort);
		//confirm that the file exists
		if(access(fileName, F_OK) != -1){
			printf("Sending \"%s'\" to %s:%d\n\n", fileName, clientName, dataPort);
			memset(buffer, '\0', MAX_CHARS_MESSAGE);
			sprintf(buffer, "Receiving \"%s\" from %s:%d", fileName, serverName, dataPort);
		} else {
			type = 'n';
			printf("File not found. Sending error message to %s:%d\n\n", clientName, dataPort);
			memset(buffer, '\0', MAX_CHARS_MESSAGE);
			sprintf(buffer, "%s:%d says FILE NOT FOUND", serverName, controlPort);
		}	
	}
	
	//send message to client on control connection 
	int charsWritten = 0;
	do{
		charsWritten += send(controlConnection, buffer, strlen(buffer), 0); //write to socket
		if(charsWritten < 0){
			fprintf(stderr, "Error writing to socket.\n");
			exit(2); 
		}
	} while(charsWritten < strlen(buffer));
	
	//transfer the file
	fileTransfer(buffer, type, dataPort, fileName);
}

/* Function: fileTransfer
 * --------------------------
 *  Creates a data connection.
 *  Sends a file to the client and 
 *  then closes the data connection.
 *
 *  buffer: A buffer that holds file info or listing info to be sent to the client.
 *  type: The type of request sent.
 *  dataPort: The port number for the data port.
 *  fileName: The name of the requested file. 
 */
void fileTransfer(char *buffer, char type, int dataPort, char *fileName)
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
	
	//accept the next connection
	size_client_info = sizeof(clientAddress); //get the size of the address for the client that will connect
	dataConnection = accept(listen_socket, (struct sockaddr *)&clientAddress, &size_client_info); //accept
	if(dataConnection < 0){
		fprintf(stderr, "Error on data accept\n");
		return;	
	}
	
	//get data that needs to be sent
	if(type == 'l'){
		//get the contents of current directory and put it into the buffer
		char *bufferPtr = buffer;
		struct dirent *dir_entry;
		DIR *dir = opendir(".");
		while((dir_entry = readdir(dir)) != NULL) {
			bufferPtr += sprintf(bufferPtr, "%s ", dir_entry->d_name);
		}
	} else if (type == 'g'){
		//get the contents of file and put it into the buffer
		memset(buffer, '\0', MAX_CHARS_MESSAGE);
		
		//open the file
		FILE *fp = fopen(fileName, "r");
		if(!fp) fprintf(stderr, "Error could not open file");

		//get the size of the file
		int fileSize;
		fseek(fp, 0L, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		//read the file into buffer
		fread(buffer, sizeof(char), fileSize, fp);
		//add a special end of file string
		sprintf(buffer + strlen(buffer), "!$@$!");

		//close the file
		fclose(fp);
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
