#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <pthread.h>
#define PRINT_ERROR printf
int main(int argc, char ** argv)
{
	struct sockaddr_in address;
	char buffer[1024] = {0};
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

	status = listen(sockid, 3);
	if(status < 0)
		PRINT_ERROR("Error on Listen\n");

	int newSocket = accept(sockid, (struct sockaddr*) &address, (socklen_t*)&addrlen);
	if(newSocket < 0)
		PRINT_ERROR("Error on Accept");
	else
		printf("Success on connection to client!\n");

	int readFrom = read( newSocket , buffer, 1024); 
   	printf("%s\n",buffer);
    	send(newSocket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 ); 

	status = close(sockid);
	if(status < 0)
		PRINT_ERROR("Error on Close\n");

	return 0;
}
