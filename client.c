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
#include<netdb.h>

#include "fileHelperMethods.h"


/**
writes .configure file in directory of executable with IP and PORT info
**/
bool writeConfigureFile(char* IP, char* port){
	//CHECK ARGUMENTS
	if( gethostbyname(IP) == NULL ) pRETURN_ERROR("issue reading IP",false); 
	int pnum = (int)strtol(port, NULL, 10);
	if(pnum<8000 ||pnum>65535) pRETURN_ERROR("port must be an int between 8000 and 65535", false);
	
	
	//CREATE CONF FILE
	int file = openFileW("./.configure");
		if(file<0) pRETURN_ERROR("writing configure file", false); 
	
	//WRITE IP and Port info to file
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
		if(conf==NULL) pRETURN_ERROR("error reading file, did not configure",false);
	
	//SET IP AND PORT
	*IP_addr = strtok(conf, "\n");
	*PORT_addr = (int)strtol(strtok(NULL, "\n"), NULL, 10);
	
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
	char* command = arg[1];
	
	//MAKE CONFIGURE FILE
	if(strcmp(command,"configure")==0){
		if(argc!=4) pRETURN_ERROR("configure must be followed by 2 arguments: IP and Port", -1);
		return writeConfigureFile(argv[2], argv[3])? 0: -1;//write config file and return
	}
	
	//CONNECT TO SERVER
		connectToServer();
		
	//COMMANDS
	//checkout
	if (strcmp(command,"checkout")==0){
		if(argc!=3) pRETURN_ERROR("checkout must be followed by 1 argument: project name", -1);
	
	//update
	}else if (strcmp(command,"update")==0){
		if(argc!=3) pRETURN_ERROR("update must be followed by 1 argument: project name", -1);
		
	//upgrade
	}else if (strcmp(command,"upgrade")==0){
		if(argc!=3) pRETURN_ERROR("upgrade must be followed by 1 argument: project name", -1);
	
	//commit
	}else if (strcmp(command,"commit")==0){
		if(argc!=3) pRETURN_ERROR("commit must be followed by 1 argument: project name", -1);
		
	//push
	}else if (strcmp(command,"push")==0){
		if(argc!=3) pRETURN_ERROR("push must be followed by 1 argument: project name", -1);
		
	//create
	}else if (strcmp(command,"create")==0){
		if(argc!=3) pRETURN_ERROR("push must be followed by 1 argument: project name", -1);
		
	//destroy
	}else if (strcmp(command,"destroy")==0){
		if(argc!=3) pRETURN_ERROR("push must be followed by 1 argument: project name", -1);
		
	//add
	}else if (strcmp(command,"add")==0){
		
	//remove
	}else if (strcmp(command,"remove")==0){
	
	//currentversion
	}else if (strcmp(command,"currentversion")==0){
		
	//history
	}else if (strcmp(command,"history")==0){
	
	//rollback	
	}else if (strcmp(command,"rollback")==0){
		
	}else{
		pRETURN_ERROR("did not enter in a valid command (refrence ReadMe for valid commands)",-1);
	}
		
	return 0;
}
