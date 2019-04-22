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

//GLOBAL
int sockfd = -1;


//CONFIGURE//////////////////////////////////////////////////////////////
/**
writes .configure file in directory of executable with IP and PORT info
**/
bool writeConfigureFile(char* IP, char* port){
	//CHECK ARGUMENTS
	//checking IP
		if( gethostbyname(IP) == NULL ) pRETURN_ERROR("issue reading IP",false); 
	//checking port
		int pnum = (int)strtol(port, NULL, 10); 
		if(pnum<8000 ||pnum>65535) pRETURN_ERROR("port must be an int between 8000 and 65535", false);
		
	//CREATE CONFIGURE FILE
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
/////////////////////////////////////////////////////////////////////////



//[3.1] CHECKOUT//////////////////////////////////////////////////////////
bool checkoutClient(char* proj_name){
	char buffer [BUFSIZ];
	write(sockfd, "checkout", strlen("checkout"));
	read(sockfd, buffer, BUFSIZ);
	write(sockfd, proj_name , strlen(proj_name));
	recv(sockfd, buffer, BUFSIZ, 0);
	int file_size = (int)atol(buffer);
	int remain_data = file_size;	
	//while((remain_data>0) && (())
	return true;
}
////////////////////////////////////////////////////////////////////////



//[3.2] UPDATE//////////////////////////////////////////////////////////////
bool updateClient(char* proj_name){
	return true;
}
///////////////////////////////////////////////////////////////////////////



//[3.3] UPGRADE//////////////////////////////////////////////////////////////
bool upgradeClient(char* proj_name){
	return true;
}
////////////////////////////////////////////////////////////////////////////



//[3.4] COMMIT//////////////////////////////////////////////////////////////
bool commitClient(char* proj_name){
	return true;
}
////////////////////////////////////////////////////////////////////////////



//[3.5] PUSH//////////////////////////////////////////////////////////////
bool pushClient(char* proj_name){
	return true;
}
/////////////////////////////////////////////////////////////////////////



//[3.6] CREATE//////////////////////////////////////////////////////////////
bool createClient(char* proj_name){
	write(sockfd, "create", strlen("create"));
	write(sockfd, proj_name , strlen(proj_name));
	return true;
}
////////////////////////////////////////////////////////////////////////////



//[3.7] DESTROY//////////////////////////////////////////////////////////////
bool destroyClient(char* proj_name){
	return true;
}
////////////////////////////////////////////////////////////////////////////



//[3.8] ADD//////////////////////////////////////////////////////////////
bool addClient(char* proj_name, char* file_name){
	return true;
}
////////////////////////////////////////////////////////////////////////



//[3.9] REMOVE//////////////////////////////////////////////////////////////
bool removeClient(char* proj_name, char* file_name){
	return true;
}
////////////////////////////////////////////////////////////////////////////



//[3.10] CURRENT VERSION/////////////////////////////////////////////////////
bool currentVersionClient(char* proj_name){
	return true;
}
////////////////////////////////////////////////////////////////////////////



//[3.11] HISTORY//////////////////////////////////////////////////////////////
bool historyClient(char* proj_name){
	return true;
}
//////////////////////////////////////////////////////////////////////////////



//[3.12] ROLLBACK//////////////////////////////////////////////////////////////
bool rollbackClient(char* proj_name){
	return true;
}
//////////////////////////////////////////////////////////////////////////////




//CONNECT//////////////////////////////////////////////////////////////

/**
connects to server using the "server.configure" file  TODO print out statements
**/
bool connectToServer(int sockfd){
	//initialize IP and PORT_addr
		char* IP;
		int PORT;
		if(initializeIPandPort(&IP, &PORT)==false) return false;
		
	//declaring vars
		//struct sockaddr_in sock_addr;
		struct sockaddr_in serv_addr;
		
	//get server address
		bzero(&serv_addr, sizeof(serv_addr));
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
////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv){
	if(argc<2) pRETURN_ERROR("Not enough arguments", -1);
	
	char* command = argv[1];
	
	//initialize socket
	if( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		pRETURN_ERROR("Socket Creation", false);
	
	//MAKE CONFIGURE FILE
	if(strcmp(command,"configure")==0){
		if(argc!=4) pRETURN_ERROR("configure must be followed by 2 arguments: IP and Port", -1);
		//write config file and return
		return writeConfigureFile(argv[2], argv[3])? 0: -1;
	}
	
	//CONNECT TO SERVER
		connectToServer(sockfd);

	//COMMANDS
	//[3.1] checkout
	if (strcmp(command,"checkout")==0){
		if(argc!=3) pRETURN_ERROR("checkout must be followed by 1 argument: project name", -1);
		return checkoutClient(argv[2])? 0: -1;
		
	//[3.2] update
	}else if (strcmp(command,"update")==0){
		if(argc!=3) pRETURN_ERROR("update must be followed by 1 argument: project name", -1);
		return updateClient(argv[2])? 0: -1;
		
	//[3.3] upgrade
	}else if (strcmp(command,"upgrade")==0){
		if(argc!=3) pRETURN_ERROR("upgrade must be followed by 1 argument: project name", -1);
		return upgradeClient(argv[2])? 0: -1;
		
	//[3.4] commit
	}else if (strcmp(command,"commit")==0){
		if(argc!=3) pRETURN_ERROR("commit must be followed by 1 argument: project name", -1);
		return commitClient(argv[2])? 0: -1;
		
	//[3.5] push
	}else if (strcmp(command,"push")==0){
		if(argc!=3) pRETURN_ERROR("push must be followed by 1 argument: project name", -1);
		return pushClient(argv[2])? 0: -1;
		
	//[3.6] create
	}else if (strcmp(command,"create")==0){
		if(argc!=3) pRETURN_ERROR("create must be followed by 1 argument: project name", -1);
		return createClient(argv[2])? 0: -1;
		
	//[3.7] destroy
	}else if (strcmp(command,"destroy")==0){
		if(argc!=3) pRETURN_ERROR("destroy must be followed by 1 argument: project name", -1);
		return destroyClient(argv[2])? 0: -1;
		
	//[3.8] add
	}else if (strcmp(command,"add")==0){
		if(argc!=4) pRETURN_ERROR("destroy must be followed by 2 arguments: project name and file name", -1);
		return addClient(argv[2], argv[3])? 0: -1;
		
	//[3.9] remove
	}else if (strcmp(command,"remove")==0){
		if(argc!=4) pRETURN_ERROR("remove must be followed by 2 arguments: project name and file name", -1);
		return removeClient(argv[2], argv[3])? 0: -1;
		
	//[3.10] currentversion
	}else if (strcmp(command,"currentversion")==0){
		if(argc!=3) pRETURN_ERROR("currentversion must be followed by 1 argument: project name", -1);
		return currentVersionClient(argv[2])? 0: -1;
		
	//[3.11] history
	}else if (strcmp(command,"history")==0){
		if(argc!=3) pRETURN_ERROR("history must be followed by 1 argument: project name", -1);
		return historyClient(argv[2])? 0: -1;
		
	//[3.12] rollback	
	}else if (strcmp(command,"rollback")==0){
		if(argc!=3) pRETURN_ERROR("rollback must be followed by 1 argument: project name", -1);
		return rollbackClient(argv[2])? 0: -1;
		
	}else{
		pRETURN_ERROR("did not enter in a valid command (refrence ReadMe for valid commands)",-1);
	}		
		
		
		
	return 0;
}
