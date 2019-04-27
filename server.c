#include <stdio.h>
//#include <iostream.h>
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

#include "fileHelperMethods.h"

//TODO: put in header file
//Struct for linked list of ip adresses of pthreads being created
typedef struct ProjectNode{
	char* project_name;
	struct ProjectNode *next;
}ProjectNode;


typedef struct ClientThread{
	pthread_t client;
	int curr_socket;
}ClientThread;



//GLOBALS////////////////////////////////////////////////////////////////
int socket_c;

ClientThread clients[20];
int num_clients = 0;
/////////////////////////////////////////////////////////////////////////

//Method to make a tar file of project to send over to client
char* makeTar(char* proj_name, char* path_File){

	printf("proj_name: %s\n", proj_name);

	//Find Path of parent of current file
	int i=0;
	int pos = strlen(path_File);
	int num = 0;
	for(i=strlen(path_File)-1; i>=0; i--){
		if(num==2)
			break;
		if(path_File[i] == '/')
			num++;
		pos--;
	}

	//Path of parent file
	char* path_Store = substr(path_File, 0, pos+2);

	int size_Path = strlen(path_Store) + strlen(proj_name) + strlen(".tgz")+1;
	int size_New = strlen("tar czf ")+ size_Path + 1 + strlen(proj_name)+1;
	
	//Create return of new path for tar'd project
	char* ret = (char*)malloc(size_Path* sizeof(char));
	strcpy(ret, path_Store);
	strcat(ret, proj_name);
	strcat(ret,".tgz");
	
	
	//Create the Tar command and run the command
	char command[size_New];
	strcpy(command, "tar czf ");
	strcat(command, ret);
	strcat(command, " ");
	strcat(command, proj_name);
	system(command);

	return ret;
}

//checkout///////////////////////////////////////////////////////////////////////
//Accepts project Name as an incoming request from client and sends back current version of project
void* checkoutServer( int curr_sockid, char* proj_name ){

	//Find path of file of project name	
	char* path_File = realpath(proj_name,NULL);
	//if(path_File==NULL) pRETURN_ERROR("File does not exist or memory has overloaded",-1); //TODO returns an int

	char* to_read = makeTar(proj_name, path_File);

	char* to_send = readFile(to_read);

	return 0;
}




void* updateServer(  int curr_sockid, char* proj_name  ){

	return 0;
}




void* upgradeServer(  int curr_sockid, char* proj_name  ){

	return 0;
}




void* commitServer( int curr_sockid, char* proj_name ){


	return 0;
}



void* pushServer(  int curr_sockid, char* proj_name  ){

	return 0;
}




void* createServer(  int curr_sockid, char* proj_name ){

	return 0;
}


void* destroyServer(  int curr_sockid, char* proj_name  ){

	return 0;
}


void* addServer(  int curr_sockid, char* proj_name, char* file_name ){

	return 0;
}


void* removeServer( int curr_sockid, char* proj_name, char* file_name  ){

	return 0;
}


void* currentversionServer(  int curr_sockid, char* proj_name  ){

	return 0;
}


void* historyServer( int curr_sockid, char* proj_name  ){

	return 0;
}


////////////////////////////////////////////////////////////////////////
void* rollbackServer(  int curr_sockid, char* proj_name, char* version_num){

	return 0;
}
////////////////////////////////////////////////////////////////////////



//CONNECT//////////////////////////////////////////////////////////////////////
//Handles accepting information sent in by the client
void* connect_client(void* curr_socket ){
	int curr_sockid = *(int*)curr_socket;

	//Recieve number of bytes to read from client
	int num_bytes;
	READ_AND_CHECKn( curr_sockid , &num_bytes, 4);
	printf("recieved from client - num_bytes_toread: %d\n", num_bytes );

	//Recieve info from client
	char* info_from_client = (char*)malloc( num_bytes +1);
			if( info_from_client==NULL ) pEXIT_ERROR("malloc");
	READ_AND_CHECKn( curr_sockid , info_from_client , num_bytes);
	info_from_client[num_bytes] = '\0';
	printf("recieved from client - info_from_client: %s\n", info_from_client);


	//Parse through info
	char* command = strtok(info_from_client, ":");
	char* project = strtok(NULL, ":");
	char* s3 = strtok(NULL, ":"); //file or version_num if applicable



	printf("Success on connection to client!\n");


	//The following if statements call methods based on the request sent from the client
	if(strcmp(command,"checkout")==0)
		checkoutServer(curr_sockid, project);

	else if(strcmp(command,"update")==0)
		updateServer(curr_sockid, project);

	else if(strcmp(command,"upgrade")==0)
		upgradeServer(curr_sockid, project);

	else if(strcmp(command,"commit")==0)
		commitServer(curr_sockid, project);

	else if(strcmp(command,"push")==0)
		pushServer(curr_sockid, project);

	else if(strcmp(command,"create")==0)
		createServer(curr_sockid, project);

	else if(strcmp(command,"destroy")==0)
		destroyServer(curr_sockid, project);

	else if(strcmp(command,"add")==0)
		addServer(curr_sockid, project, s3);

	else if(strcmp(command,"remove")==0)
		removeServer(curr_sockid, project, s3);

	else if(strcmp(command,"currentversion")==0)
		currentversionServer(curr_sockid, project);

	else if(strcmp(command,"history")==0)
		historyServer(curr_sockid, project);

	else if(strcmp(command,"rollback")==0)
		rollbackServer(curr_sockid, project, s3);

	//TODO delete(use as reference for methods)
	//send to socket
	//send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 );

	//freeing and exiting
	/*
	shutdown(socket_c,0);
	shutdown(socket_c,1);
	shutdown(socket_c,2);
	*/
	//if(close(socket_c) < 0) pRETURN_ERROR("Error on Close", NULL);
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
