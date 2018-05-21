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
void receive_message(int connection);

int main(int argc, char *argv[])
{
	//make sure the user passed valid arguments
	if(!check_args(argc, argv)){
		return 0;
	}

	//set port number	
	int port_num = strtol(argv[1], NULL, 10);

	//setup the  server and start listening
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
	//close the connection
	receive_message(connection);
	close(connection);
	printf("Closed connection\n");
}

/* Function: send_message
 * --------------------------
 *  User either starts by typing a message to 
 *  the server, or user types "\quit" to end the chat.
 *
 *  socketFD: The socket number.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 *
void send_message(int socketFD, char *handle, size_t handle_size)
{
	//setup variables
	int chars_written;
	char buffer[MAX_CHARS_MESSAGE + MAX_CHARS_HANDLE + 4];
	char message[MAX_CHARS_MESSAGE + 1];
	memset(buffer, '\0', MAX_CHARS_MESSAGE + MAX_CHARS_HANDLE + 4);
	memset(message, '\0', MAX_CHARS_MESSAGE + 1);

	//get message from user
	printf("%s> ", handle); //print a prompt for the user
	fflush(stdout); //make sure we printed the prompt
	fgets(message, sizeof(message), stdin); //get a message from the user
		
	//remove newline from message, if there is none clear stdin until we find one
	if(message[strlen(message)-1] == '\n'){
		strtok(message, "\n"); //remove newline
	} else {
		char c;
		while((c = getchar()) != '\n' && c != EOF); //clear stdin
	}

	//see if we are quiting. if we are let server know
	if(!strcmp(message, "\\quit")){
		send(socketFD, "\\quit\n", strlen("\\quit\n"), 0);	
		exit(0);
	}

	snprintf(buffer, sizeof(buffer), "%s> %s\n", handle, message); //add our handle to the message

	//send message to server
	chars_written = 0;
	do{
		chars_written += send(socketFD, buffer, strlen(buffer), 0); //write to socket
		if(chars_written < 0){
			fprintf(stderr, "Error writing to socket.\n");
			exit(2); 
		}
	} while(chars_written < strlen(buffer));
}
*/

/* Function: receive_message
 * --------------------------
 *  Read a message from the client.
 *
 *  connection: The connection number.
 */
void receive_message(int connection)
{
	//setup variables
	int chars_read;
	char buffer[MAX_CHARS_MESSAGE + 1];
	memset(buffer, '\0', MAX_CHARS_MESSAGE + 1);
	
	//read message from the client
	chars_read = 0;
	int bufLen; //holds the buffer length
	int bufSum = 0; //the number of chars we have writen to our buffer
	do{
		chars_read = recv(connection, &buffer[bufSum], 100, 0); //read from socket
		bufSum += chars_read;
		bufLen = strlen(buffer);
		if(chars_read < 0){
			fprintf(stderr, "Error reading from socket.\n");
			exit(2); 
		}
	} while(buffer[bufLen - 1] != '\n');

	//if server is quiting we also quit
	if(!strcmp(buffer, "\\quit\n")){
		exit(0);
	}

	//show message from server
	printf("%s", buffer); 
}
