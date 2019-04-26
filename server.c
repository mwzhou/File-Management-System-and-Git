#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#include "server.h"


//GLOBALS////////////////////////////////////////////////////////////////
int socket_c;

ClientThread clients[20];
int num_clients = 0;
/////////////////////////////////////////////////////////////////////////


//[3.1] CHECKOUT////////////////////////////////////////////////////////////////////////

/*
Accepts project name as an incoming request from client and sends back current version of project
*/
void* checkoutServer( int curr_sockid, char* proj_name ){
	printf("%d] Entered command: checkout\n", curr_sockid);
	return 0;
}
////////////////////////////////////////////////////////////////////////


//[3.2] UPDATE//////////////////////////////////////////////////////////////////////

/*
update
*/
void* updateServer(  int curr_sockid, char* proj_name  ){
	printf("%d] Entered command: update\n", curr_sockid);

	//error check
		if( typeOfFile(proj_name)!=isDIR ){ sendErrorSocket(curr_sockid); pRETURN_ERROR("project doesn't exist on server",NULL); }
		if( typeOfFile(".Manifest") != isREG ){  sendErrorSocket(curr_sockid); printf("error\n"); pRETURN_ERROR(".Manifest file doesn't exist on server",NULL); }

	//send manifest file to client
		char* manifest_str = readFile(".Manifest"); //TODO: tar
			if(manifest_str == NULL){ sendErrorSocket(curr_sockid); return NULL; }
		sendFile( curr_sockid, manifest_str);

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* upgradeServer(  int curr_sockid, char* proj_name  ){
	//error check
		if( typeOfFile(proj_name)!=isDIR ){ sendErrorSocket(curr_sockid); pRETURN_ERROR("project doesn't exist on server",NULL); }

	//TODO:
		int num_bytes;
		READ_AND_CHECKn(curr_sockid, &num_bytes, 4);
			if(num_bytes<=0){ pRETURN_ERROR("Error on Server side", NULL); }

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* commitServer( int curr_sockid, char* proj_name ){


	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* pushServer(  int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* createServer(  int curr_sockid, char* proj_name ){

	return 0;
}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
void* destroyServer(  int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* addServer(  int curr_sockid, char* proj_name, char* file_name ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* removeServer( int curr_sockid, char* proj_name, char* file_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* currentversionServer(  int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* historyServer( int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* rollbackServer(  int curr_sockid, char* proj_name, char* version_num){

	return 0;
}
////////////////////////////////////////////////////////////////////////



//CONNECT//////////////////////////////////////////////////////////////////////
//Handles accepting information sent in by the client
void* connect_client(void* curr_socket ){
	int curr_sockid = *(int*)curr_socket;
	printf("%d] Success on connection to client %d!\n", curr_sockid, num_clients);

	//Recieve number of bytes to read from client
		int num_bytes;
		READ_AND_CHECKn( curr_sockid , &num_bytes, 4);
			//if error
			if(num_bytes <= 0){ printf("\tError on client side\n"); return 0; }
		printf("\tRecieved from client - num_bytes_toread: %d\n", num_bytes );


	//Recieve info from client
		char* info_from_client = (char*)malloc(num_bytes + 1);
				if( info_from_client==NULL ) pEXIT_ERROR("malloc");
		READ_AND_CHECKn( curr_sockid , info_from_client , num_bytes);
		info_from_client[num_bytes] = '\0';

	//Parse through info and store in variables
		char delim[1]; delim[0] = (char)176;
		char* command = strtok(info_from_client, delim);
		char* proj_name = strtok(NULL, delim);
		char* s3 = strtok(NULL, delim); //file or version_num if applicable
		printf("\tRecieved from client - command:%s  proj_name:%s  extra_info?:%s\n", command, proj_name, s3);




	//The following if statements call methods based on the request sent from the client
	if(strcmp(command,"checkout")==0)
		checkoutServer(curr_sockid, proj_name);

	else if(strcmp(command,"update")==0)
		updateServer(curr_sockid, proj_name);

	else if(strcmp(command,"upgrade")==0)
		upgradeServer(curr_sockid, proj_name);

	else if(strcmp(command,"commit")==0)
		commitServer(curr_sockid, proj_name);

	else if(strcmp(command,"push")==0)
		pushServer(curr_sockid, proj_name);

	else if(strcmp(command,"create")==0)
		createServer(curr_sockid, proj_name);

	else if(strcmp(command,"destroy")==0)
		destroyServer(curr_sockid, proj_name);

	else if(strcmp(command,"add")==0)
		addServer(curr_sockid, proj_name, s3);

	else if(strcmp(command,"remove")==0)
		removeServer(curr_sockid, proj_name, s3);

	else if(strcmp(command,"currentversion")==0)
		currentversionServer(curr_sockid, proj_name);

	else if(strcmp(command,"history")==0)
		historyServer(curr_sockid, proj_name);

	else if(strcmp(command,"rollback")==0)
		rollbackServer(curr_sockid, proj_name, s3);

	else
		printf("\tError on client side\n");

	//TODO delete(use as reference for methods)
	//send to socket
	//send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 );

	//freeing and exiting
	/*
	shutdown(socket_c,0);
	shutdown(socket_c,1);
	shutdown(socket_c,2);
	*/

	//close
	if(close(socket_c) < 0) pRETURN_ERROR("Error on Close", NULL);
	//pthread_exit(NULL);

	return 0;
}
////////////////////////////////////////////////////////////////////////



int main(int argc, char * argv[]){ //TODO: print out error message?
	//Check for arguments
	if(argc!=2) pRETURN_ERROR("Enter an argument containing the port number\n",-1);

	//getting port
	int port = (int)atol(argv[1]);

	//server address
	struct sockaddr_in address;

	//CREATING SOCKET
	socket_c = socket(AF_INET, SOCK_STREAM, 0);
		if(socket_c == 0) pRETURN_ERROR("Error on socket creation",-1);
	//reuse socket
	if (setsockopt(socket_c, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)  pRETURN_ERROR("setsockopt(SO_REUSEADDR) failed",-1);

	//initialize socket_c


	//intitializes sizeof(address) zero-value bytes of address
	bzero(&address,sizeof(address));

	//initializes sockaddr_in functions for address
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	//BINDING to client
	int addrlen = sizeof(address);
	int status = bind(socket_c, (struct sockaddr*) &address, addrlen);
		if(status < 0) pRETURN_ERROR("Error on Bind",-1);

	//LISTENto client TODO: change to 20
	status = listen(socket_c, 20);
		if(status < 0) pRETURN_ERROR("Error on Listen",-1);


	//ACCEPT connecting and accepting message for client
	int curr_socket;
	while( (curr_socket= accept(socket_c, (struct sockaddr*) &address, (socklen_t*)&addrlen)) >0 ){
	 //add client thread to global
	 	ClientThread curr_client = {-1, curr_socket};
		clients[num_clients] =  curr_client; //TODO: change -1 to pthread

		num_clients++; //TODO: add mutex
		//connect to client
		connect_client( (void*)&curr_socket );
	}

	//if accept failed
	if(curr_socket<0) pRETURN_ERROR("Connection to client failed",-1);
/*SHOULD BE INSIDE LOOP
		//create new thread
		pthread_t id;
		status = pthread_create(&id, NULL, connect_client, (void*) &tempSocket);
			if(status<0) pRETURN_ERROR("Thread not created",-1);
		//insert id into linked list
		insert(id);
	}
*/

/*
	//SHUT DOWN AND RETURN
	shutdown(socket_c,0);
	shutdown(socket_c,1);
	shutdown(socket_c,2);
*/
	if(close(socket_c) < 0) pRETURN_ERROR("Error on Close",-1);

	return 0;
}
