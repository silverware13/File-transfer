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

//function prototypes
bool check_args(int argc, char *argv[]);
void start_up(int port_num);
void handle_command(int connection);
void file_transfer(int connection, char *buffer);

int main(int argc, char *argv[])
{
	//make sure the user passed valid arguments
	if(!check_args(argc, argv)){
		return 0;
	}

	//set port number	
	int port_num = strtol(argv[1], NULL, 10);

	//setup the  server and handle all incoming commands
	start_up(port_num);
	
	return 0;
}

/* Function: check_args
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
bool check_args(int argc, char *argv[])
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

/* Function: start_up
 * --------------------------
 *  Starts up the server and listens
 *  for connections on the given port
 *  number.
 *
 *  port_num: The port number given on command line.
 */
void start_up(int port_num)
{
	//setup variables
	int socketFD, listen_socket, connection;
	struct sockaddr_in server_address, client_address;
	struct hostent* server_host_info;
	socklen_t size_client_info;	

	//set up the server address struct
	memset((char*)&server_address, '\0', sizeof(server_address)); //clear address struct
	server_address.sin_family = AF_INET; //set address family
	server_address.sin_port = htons(port_num); //save port number
	server_address.sin_addr.s_addr = INADDR_ANY; //we allow connection from any address

	//set up socket
	listen_socket = socket(AF_INET, SOCK_STREAM, 0); //create the socket

	//start listening with current socket for any incoming connections
	bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)); //connect socket to port
	listen(listen_socket, 10); //socket is now listening for a connection

	printf("Server open on %d\n", port_num);	
	
	//we will keep looking for connections until we are terminated
	while(1){
		//accept the next connection
		size_client_info = sizeof(client_address); //get the size of the address for the client that will connect
		connection = accept(listen_socket, (struct sockaddr *)&client_address, &size_client_info); //accept
		printf("Connection from %s\n", inet_ntop(AF_INET,&client_address.sin_addr,
                    client_address,sizeof(client_address)));
		//react to clients command
		handle_command(connection);
	}
}

/* Function: handle_command
 * --------------------------
 *  Receive a command from the client
 *  and then send an appropriate response.
 *
 *  connection: The connection number.
 */
void handle_command(int connection)
{
	//setup variables
	int bufLen; //holds the buffer length
	int bufSum = 0; //the number of chars we have writen to our buffer
	int chars_read = 0;
	char buffer[MAX_CHARS_MESSAGE + 1];
	memset(buffer, '\0', MAX_CHARS_MESSAGE + 1);
	
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

	//show message from server
	file_transfer(connection, buffer); 
	close(connection);
}

/* Function: file_transfer
 * --------------------------
 *  User either starts by typing a message to 
 *  the server, or user types "\quit" to end the chat.
 *
 *  socketFD: The socket number.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 */
void file_transfer(int connection, char *buffer)
{
	printf("THIS IS THE MESSAGE: %s\n", buffer);
	//setup variables
	int bufLen, chars_written, chars_read, list_mode;
	char portBuffer[100];
	int bufSum = 0; //the number of chars we have writen to our buffer
	
	//get the data port from the request
	int i = 0;
	do{
		portBuffer[i] = buffer[i];
		bufLen = strlen(portBuffer);
		i++;
	} while(portBuffer[bufLen - 2] != '-');

	//see if the request asks for list or file
	if(portBuffer[bufLen - 1] == 'l')
	{
		list_mode = true;
	} else {
		list_mode = false;
	}
	
	//remove the - from the portBuffer
	portBuffer[bufLen - 2] = '\0';
	int dataPort = strtol(portBuffer, NULL, 10);
	printf("THIS IS THE PORT: %d\n", dataPort);
	
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

	if(list_mode){
		printf("List directory requested on port %d\n", dataPort);	
	} else {
		printf("File “shortfile.txt” requested on port %d\n", dataPort);	
	}

	//accept the next connection
	size_client_info = sizeof(client_address); //get the size of the address for the client that will connect
	data_connection = accept(listen_socket, (struct sockaddr *)&client_address, &size_client_info); //accept

	//perform the correct action
	if(list_mode){
		printf("Sending directory contents to %d\n", dataPort);	
	} else {
		printf("Sending 'shortfile.txt' to %d\n", dataPort);	
	}

	//send file to client
	chars_written = 0;
	do{
		chars_written += send(data_connection, buffer, strlen(buffer), 0); //write to socket
		if(chars_written < 0){
			fprintf(stderr, "Error writing to socket.\n");
			exit(2); 
		}
	} while(chars_written < strlen(buffer));
}
