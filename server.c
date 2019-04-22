#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdbool.h>
#include <errno.h>

#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h>
#include <pthread.h>

#include "fileHelperMethods.h"

//Struct for linked list of ip adresses of pthreads being created
typedef struct pt_Node{
	int id;
	struct pt_Node *next;
}pt_Node;

pt_Node* head = NULL;
pthread_mutex_t lock;

//Accepts project Name as an incoming request from client and sends back current version of project
void* checkoutServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* updateServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* upgradeServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* commitServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* pushServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
		if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* createServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* destroyServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* addServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);

	//Accept file Name from Client
	char file_Name[50] = {0};
	readFrom = read( socket , file_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);

	return 0;
}

void* removeServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);

	//Accept file Name from Client
	char file_Name[50] = {0};
	readFrom = read( socket , file_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);

	return 0;
}

void* currentversionServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* historyServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	return 0;
}

void* rollbackServer(int socket){
	
	//Accept project Name from Client
	char project_Name[50] = {0};
	int readFrom = read( socket , project_Name, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);

	//Accept version number from Client
	char version_Num[50] = {0};
	readFrom = read( socket , version_Num, 50); 
	if(readFrom<0) pRETURN_ERROR("read", NULL);
	int version = (int)atol(version_Num);
	printf("%d\n",version);//TODO delete

	return 0;
}

//Inserts IP addresses of each pthread into a node to allow us to close each of them individually at the end
void insert(int ipAddress){
	pt_Node *insert_info = (pt_Node*)malloc(sizeof(pt_Node));
	insert_info->id = ipAddress;
	insert_info->next = NULL;
	if(head==NULL)
		head = insert_info;
	else{
		pt_Node *temp = head;
		while(temp->next!=NULL)
			temp = temp->next;
		temp->next = insert_info;
	}
}

//Handles accepting information sent in by the client
void* connect_client(void *sockid){
	//create socket
	int socket = *(int*)sockid;

	//READ from client
	char buffer_Client[50] = {0};

	int readFrom = read( socket , buffer_Client, 50); 
		if(readFrom<0) pRETURN_ERROR("read", NULL);

	//The following if statements call methods based on the request sent from the client	
	if(strcmp(buffer_Client,"checkout")==0)
		checkoutServer(socket);
	else if(strcmp(buffer_Client,"update")==0)
		updateServer(socket);
	else if(strcmp(buffer_Client,"upgrade")==0)
		upgradeServer(socket);
	else if(strcmp(buffer_Client,"commit")==0)
		commitServer(socket);
	else if(strcmp(buffer_Client,"push")==0)
		pushServer(socket);
	else if(strcmp(buffer_Client,"create")==0)
		createServer(socket);
	else if(strcmp(buffer_Client,"destroy")==0)
		destroyServer(socket);
	else if(strcmp(buffer_Client,"add")==0)
		addServer(socket);
	else if(strcmp(buffer_Client,"remove")==0)
		removeServer(socket);
	else if(strcmp(buffer_Client,"currentversion")==0)
		currentversionServer(socket);
	else if(strcmp(buffer_Client,"history")==0)
		historyServer(socket);
	else if(strcmp(buffer_Client,"rollback")==0)
		rollbackServer(socket);

	//TODO delete(use as reference for methods)	
	//send to socket
	//send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 ); 

	//freeing and exiting
	shutdown(socket,0);
	shutdown(socket,1);
	shutdown(socket,2);
	if(close(socket) < 0) pRETURN_ERROR("Error on Close", NULL);
	pthread_exit(NULL);
	
	return 0;
}


int main(int argc, char * argv[]){ //TODO: print out error message?
	//Check for arguments
	if(argc!=2) pRETURN_ERROR("Enter an argument containing the port number\n",-1);
		
	//getting port
	int port = (int)atol(argv[1]);
	
	//server address
	struct sockaddr_in address;
	
	//CREATING SOCKET
	int sockid = socket(AF_INET, SOCK_STREAM, 0);
		if(sockid == 0) pRETURN_ERROR("Error on socket creation",-1);
		
	//intitializes sizeof(address) zero-value bytes of address
	bzero(&address,sizeof(address));

	//initializes sockaddr_in functions for address
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	//BINDING to client
	int addrlen = sizeof(address);
	int status = bind(sockid, (struct sockaddr*) &address, addrlen);
		if(status < 0) pRETURN_ERROR("Error on Bind",-1);

	//LISTENto client
	status = listen(sockid, 20);
		if(status < 0) pRETURN_ERROR("Error on Listen",-1);

	//ACCEPT connecting and accepting message for client
	int tempSocket;	
	while((tempSocket = accept(sockid, (struct sockaddr*) &address, (socklen_t*)&addrlen))>0){
		printf("Success on connection to client!\n");
		
		//create new thread
		pthread_t id;
		status = pthread_create(&id, NULL, connect_client, (void*) &tempSocket);
			if(status<0) pRETURN_ERROR("Thread not created",-1);
		//insert id into linked list
		insert(id);
	}
	//if accept failed
	if(tempSocket<0) pRETURN_ERROR("Connection to client failed",-1);


	//SHUT DOWN AND RETURN
	shutdown(sockid,0);
	shutdown(sockid,1);
	shutdown(sockid,2);	
	if(close(sockid) < 0) pRETURN_ERROR("Error on Close",-1);

	return 0;
}
