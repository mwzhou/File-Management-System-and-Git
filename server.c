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
	//pthread_mutex_t lock;
	struct pt_Node *next;
}pt_Node;

pt_Node* head = NULL;
pthread_mutex_t lock;

void *connect_client(void*);
void insert(int);

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

void* connect_client(void *sockid){
	//create socket
	int socket = *(int*)sockid;

	//READ from client
	char buffer_Client[2000] = {0};

	int readFrom = read( socket , buffer_Client, 2000); 
		if(readFrom<0) pRETURN_ERROR("read", NULL);
	
	printf("%s\n",buffer_Client);

	//send socket
	send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 ); 

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
	int tempSocket,*new_socket;	
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
