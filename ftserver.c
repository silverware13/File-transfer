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
void startup(int connectionPort);
void handleRequest(int connection, int connectionPort, char *client_name);
void fileTransfer(int connection, char *buffer, char type, int dataPort);

int main(int argc, char *argv[])
{
	//make sure the user passed valid arguments
	if(!checkArgs(argc, argv)){
		return 0;
	}

	//set port number	
	int connectionPort = strtol(argv[1], NULL, 10);

	//setup the  server and handle all incoming commands
	startup(connectionPort);
	
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
 *  for connections on the given port
 *  number.
 *
 *  connectionPort: The port number given on command line.
 */
void startup(int connectionPort)
{
	//setup variables
	int socketFD, listen_socket, connection;
	struct sockaddr_in server_address, client_address;
	struct hostent* server_host_info;
	socklen_t size_client_info;	

	//set up the server address struct
	memset((char*)&server_address, '\0', sizeof(server_address)); //clear address struct
	server_address.sin_family = AF_INET; //set address family
	server_address.sin_port = htons(connectionPort); //save port number
	server_address.sin_addr.s_addr = INADDR_ANY; //we allow connection from any address

	//set up socket
	listen_socket = socket(AF_INET, SOCK_STREAM, 0); //create the socket

	//start listening with current socket for any incoming connections
	bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)); //connect socket to port
	listen(listen_socket, 10); //socket is now listening for a connection

	printf("Server open on %d\n", connectionPort);	
	
	//we will keep looking for connections until we are terminated
	while(1){
		//accept the next connection
		size_client_info = sizeof(client_address); //get the size of the address for the client that will connect
		connection = accept(listen_socket, (struct sockaddr *)&client_address, &size_client_info); //accept
		
		//show the connecting client
		char client_name[INET_ADDRSTRLEN];
		memset(client_name, '\0', INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &client_address.sin_addr.s_addr, client_name, sizeof(client_name));
		printf("Connection from %s\n", client_name);

		//react to clients command
		handleRequest(connection, connectionPort, client_name);
	}
}

/* Function: handleRequest
 * --------------------------
 *  Receive a request from the client
 *  and then send an appropriate response.
 *
 *  connection: The connection number.
 */
void handleRequest(int connection, int connectionPort, char *client_name)
{
	//setup variables
	int bufLen, chars_read;
	char portBuffer[100];
	char fileName[1000];
	int bufSum = 0; //the number of chars we have writen to our buffer
	char buffer[MAX_CHARS_MESSAGE + 1];
	memset(buffer, '\0', MAX_CHARS_MESSAGE + 1);
	
	//remove the - from the portBuffer
	portBuffer[bufLen - 2] = '\0';
	int dataPort = strtol(portBuffer, NULL, 10);
	
	//read message from the client
	do{
		chars_read = recv(connection, &buffer[bufSum], 100, 0); //read from socket
		bufSum += chars_read;
		bufLen = strlen(buffer);
		if(chars_read < 0){
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
	
	//get requested file name if not list mode
	int ii = 0;
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
		printf("Sending directory contents to %s:%d\n", client_name, dataPort);	
	} else {
		printf("File \"%s\" requested on port %d\n", fileName, dataPort);
		//confirm that the file exists
		if( access( fileName, F_OK ) != -1 ) {
			printf("Sending \"%s'\" to %s:%d\n", fileName, client_name, dataPort);
		} else {
			type = 'n';
			printf("File not found. Sending error message to %s:%d\n", client_name, dataPort);
		}	
	}
	
	//transfer the file
	fileTransfer(connection, buffer, type, dataPort);

	//close the control connection 
	close(connection);
}

/* Function: fileTransfer
 * --------------------------
 *  User either starts by typing a message to 
 *  the server, or user types "\quit" to end the chat.
 *
 *  socketFD: The socket number.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 *  type: The type of request. List or get file.
 */
void fileTransfer(int connection, char *buffer, char type, int dataPort)
{
	//setup variables
	int socketFD, listen_socket, data_connection;
	struct sockaddr_in server_address, client_address;
	struct hostent* server_host_info;
	socklen_t size_client_info;	

	//set up the server address struct
	memset((char*)&server_address, '\0', sizeof(server_address)); //clear address struct
	server_address.sin_family = AF_INET; //set address family
	server_address.sin_port = htons(dataPort); //save port number
	server_address.sin_addr.s_addr = INADDR_ANY; //we allow connection from any address

	//set up socket
	listen_socket = socket(AF_INET, SOCK_STREAM, 0); //create the socket

	//start listening with current socket for any incoming connections
	bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)); //connect socket to port
	listen(listen_socket, 10); //socket is now listening for a connection


	//accept the next connection
	size_client_info = sizeof(client_address); //get the size of the address for the client that will connect
	data_connection = accept(listen_socket, (struct sockaddr *)&client_address, &size_client_info); //accept

	//if in list mode show the contents of the current directory
	if(type == 'l'){
		char *buffer_ptr = &buffer[2];
		struct dirent *dir_entry;
		DIR *dir = opendir(".");
		while((dir_entry = readdir(dir)) != NULL) {
			buffer_ptr += sprintf(buffer_ptr, "%s ", dir_entry->d_name);
		}
		sprintf(buffer_ptr, "\n\0");
		printf("%s", buffer);
	}

	//send file to client
	int chars_written = 0;
	do{
		chars_written += send(data_connection, buffer, strlen(buffer), 0); //write to socket
		if(chars_written < 0){
			fprintf(stderr, "Error writing to socket.\n");
			exit(2); 
		}
	} while(chars_written < strlen(buffer));
	close(data_connection);
}
