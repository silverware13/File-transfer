/* chatclient
 * CS372 Spring 2018
 * -----------------
 * Name: Zachary Thomas
 * Email: thomasza@oregonstate.edu
 * Date: 4/22/2018
 * -----------------
 * Connects to chatserve via a given
 * command line hostname and port number.
 * Next the user enters a handle.
 * Lastly connects to server and
 * alternates sending messages up
 * to 500 characters long.
 * -----------------------
 * Cited references:
 * Reviewed my code from my
 * CS344 OTP socket programming
 * assignment.
 * https://github.com/silverware13/OTP/blob/master/otp_enc.c 
 */

#define MAX_CHARS_MESSAGE 500
#define MAX_CHARS_HANDLE 10
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
void get_handle(char *handle, size_t handle_size);
bool initiate_contact(char *argv[], char *handle, size_t handle_size, int port_num);
void chat(int socketFD, char *handle, size_t handle_size);
void send_message(int socketFD, char *handle, size_t handle_size);
void receive_message(int socketFD, char *handle, size_t handle_size);

int main(int argc, char *argv[])
{
	//make sure the user passed valid arguments
	if(!check_args(argc, argv)){
		return 0;
	}

	//set variables	
	int port_num = strtol(argv[2], NULL, 10);
	char *handle;
	size_t handle_size = MAX_CHARS_HANDLE + 1;
	handle = (char *)calloc(MAX_CHARS_HANDLE + 1, sizeof(char));

	//get a handle for the user
	get_handle(handle, handle_size);

	//setup the connection with the server and start chat
	if(!initiate_contact(argv, handle, handle_size, port_num)){
		fprintf(stderr, "Connection failed.\n");
		return 1;
	}
	
	return 0;
}

/* Function: check_args
 * --------------------
 *  Checks to make sure there are three arguments.
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
	if(argc != 3){
		printf("useage: %s [host name] [port number]\n", argv[0]);
		return false;
	}

	//make sure the user has entered the port number as digits
	if(!isdigit(*argv[2])){
		printf("Please enter port number as an unsigned integer.\n");
		return false;
	}
	
	return true;
}

/* Function: get_handle
 * --------------------
 *  Gets a handle from the user.
 *  Must be no longer than 10 chars.
 *
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 */
void get_handle(char *handle, size_t handle_size)
{
	printf("Please enter a handle, it must be no longer than %d characters.\nHandle: ", MAX_CHARS_HANDLE);
	fflush(stdout); //make sure we printed the prompt
	fgets(handle, handle_size, stdin);

	//remove newline from handle, if there is none clear stdin until we find one
	if(handle[strlen(handle)-1] == '\n'){
		strtok(handle, "\n"); //remove newline
	} else {
		char c;
		while((c = getchar()) != '\n' && c != EOF); //clear stdin
	}

	//show the handle
	printf("Your handle is: %s\n", handle);	
}

/* Function: initiate_contact
 * --------------------------
 *  Starts connection between client
 *  and server. Then starts chat between
 *  client and server.
 *
 *  *argv[]: Array of command line arguments.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 *  port_num: The port number given on command line.
 *
 *  returns: Returns true if a connection was made and chat has completed.
 *  Returns false if a connection could not be made. 
 */
bool initiate_contact(char *argv[], char *handle, size_t handle_size, int port_num)
{
	//setup variables
	int socketFD;
	struct sockaddr_in server_address;
	struct hostent* server_host_info;
	
	//set up the server address struct
	memset((char*)&server_address, '\0', sizeof(server_address)); //clear address struct
	server_address.sin_family = AF_INET; //set address family
	server_address.sin_port = htons(port_num); //save port number
	server_host_info = gethostbyname(argv[1]); //get address
	if (server_host_info == NULL){
		return false;
	} 
	memcpy((char*)&server_address.sin_addr.s_addr, (char*)server_host_info->h_addr, server_host_info->h_length); //copy address
	
	//set up socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); //create the socket
	if (socketFD < 0){
		return false;
	}

	//connect to server
	if (connect(socketFD, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		return false;
	}

	//start chatting with the host
	chat(socketFD, handle, handle_size);
	
	return true;
}

/* Function: chat
 * --------------------------
 *  Client starts by sending a message,
 *  then we wait for the serve to send us
 *  a message. We loop until either side quits.
 *
 *  socketFD: The socket number.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 */
void chat(int socketFD, char *handle, size_t handle_size)
{
	while(true){
		//send message to server
		send_message(socketFD, handle, handle_size);	
		//read message from server
		receive_message(socketFD, handle, handle_size);	
	}
}

/* Function: send_message
 * --------------------------
 *  User either starts by typing a message to 
 *  the server, or user types "\quit" to end the chat.
 *
 *  socketFD: The socket number.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 */
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

/* Function: receive_message
 * --------------------------
 *  Read a message from the server.
 *  If we get a message letting us know
 *  that the server is quitting we terminate
 *  the program.
 *
 *  socketFD: The socket number.
 *  handle: Array that holds handle.
 *  handle_size: Size of the handle array.
 */
void receive_message(int socketFD, char *handle, size_t handle_size)
{
	//setup variables
	int chars_read;
	char buffer[MAX_CHARS_MESSAGE + MAX_CHARS_HANDLE + 4];
	memset(buffer, '\0', MAX_CHARS_MESSAGE + MAX_CHARS_HANDLE + 4);
	
	//read message from server
	chars_read = 0;
	int bufLen; //holds the buffer length
	int bufSum = 0; //the number of chars we have writen to our buffer
	memset(buffer, '\0', MAX_CHARS_MESSAGE + MAX_CHARS_HANDLE + 3);
	do{
		chars_read = recv(socketFD, &buffer[bufSum], 100, 0); //read from socket
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
