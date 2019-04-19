#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include<stdbool.h>
#include<errno.h>

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<sys/types.h>

#include "fileHelperMethods.h"


/**
writes .configure file in directory of executable with IP and PORT info
**/
bool writeConfigureFile(char* IP, char* port){
	int file = openFileW("./.configure");
		if(file<0) pRETURN_ERROR("writing configure file", false); 
	
	//write IP and Port info to file
	WRITE_AND_CHECKf(file, IP, strlen(IP));
	WRITE_AND_CHECKf(file, "\n", 1);
	WRITE_AND_CHECKf(file, port, strlen(port));
	
	close(file);
	return true;
}


/**
initializes IP and PORT by reading the configure file
**/
bool initializeIPandPort(char** IP_addr, int* PORT_addr){
	char* conf = readFile("./.configure");
		if(conf==NULL) pRETURN_ERROR("error reading file",false);
	
	//SET IP
	*IP_addr = strtok(conf, "\n");
	
	//SET PORT
	char* pstr = strtok(NULL, "\n"); //port string
	if(strcmp(pstr,"0")==0){
		*PORT_addr = 0;
	}else{
		*PORT_addr = (int)strtol(pstr, NULL, 10);
			if(*PORT_addr<=0) pRETURN_ERROR("port must be an int", false);
	}
		
	return true;
}



/**
connects to server using the "server.configure" file  TODO print out statements
**/
bool connectToServer(){
	//initialize IP and PORT_addr
		char* IP;
		int PORT;
		if(initializeIPandPort(&IP, &PORT)==false) return false;
		
	//declaring vars
		//struct sockaddr_in sock_addr;
		struct sockaddr_in serv_addr;
		int sockfd;
		
	//initialize socket
		if( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
			 pRETURN_ERROR("Socket Creation", false);
		
	//get server address
		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_port = htons(PORT); //convert to byte addr
		if(inet_pton(AF_INET, IP , &serv_addr.sin_addr)<=0) //convert to byte addr
			pRETURN_ERROR("Server Address", false);
		
	//connect to server
		if( (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0 )
			pRETURN_ERROR("Connection to Server Failure", false);
	
	printf("Successfully connected to server!\n");
	return true;		
}


int main(int argc, char** argv){
	if(argc<2) pRETURN_ERROR("Not enough arguments", -1);
		
	//MAKE CONFIGURE FILE
	if(strcmp(argv[1],"configure")==0){
		if(argc!=4) pRETURN_ERROR("configure must be followed by 2 arguments: IP and Port", -1);
		//write config file and return
		return writeConfigureFile(argv[2], argv[3])? 0: -1;
	
	}else{
		//CONNECT TO SERVER
			connectToServer();
		
		//TODO: other commands
		
	}
		
	
	return 0;
}
