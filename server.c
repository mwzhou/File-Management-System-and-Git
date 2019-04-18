#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <pthread.h>
#define PRINT_ERROR printf

void *connect_client(void*);

void *connect_client(void *sockid){

	int socket = *(int*)sockid;
	char* buffer_Client[2000] = {0};

	int readFrom = read( socket , buffer_Client, 2000); 
   	printf("%s\n",buffer_Client);
    	send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 ); 

	free(sockid);
	shutdown(socket,0);
	shutdown(socket,1);
	shutdown(socket,2);
	int status = close(socket);
	if(status < 0)
		PRINT_ERROR("Error on Close\n");
	pthread_exit(NULL);
	return 0;
}

int main(int argc, char ** argv){	
	
	struct sockaddr_in address;
	if(argc!=2){
		PRINT_ERROR("Enter an argument containing the port number\n");
		return 0;
	}
	int port = atoi(argv[1]);
	
	int sockid = socket(AF_INET, SOCK_STREAM, 0);
	if(sockid == 0)
		PRINT_ERROR("Error on socket creation\n");

	int opt =1;
	int status = setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	if(status<0)
		PRINT_ERROR("Error on Connection to port\n");

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	int addrlen = sizeof(address);
	status = bind(sockid, (struct sockaddr*) &address, addrlen);
	if(status < 0)
		PRINT_ERROR("Error on Bind\n");

	status = listen(sockid, 5);
	if(status < 0)
		PRINT_ERROR("Error on Listen\n");

	int tempSocket, *new_Sock;	
	while(tempSocket = accept(sockid, (struct sockaddr*) &address, (socklen_t*)&addrlen)){
		printf("Success on connection to client!\n");

		pthread_t id;
		new_Sock = malloc(1);
		*new_Sock = tempSocket;
		status = pthread_create(&id, NULL, connect_client, (void*) new_Sock);
		if(status<0)
			PRINT_ERROR("Thread not created\n");		
	}
	if(tempSocket<0)
		PRINT_ERROR("Connection to client failed");

	shutdown(sockid,0);
	shutdown(sockid,1);
	shutdown(sockid,2);	
	status = close(sockid);
	if(status < 0)
		PRINT_ERROR("Error on Close\n");

	return 0;
}
