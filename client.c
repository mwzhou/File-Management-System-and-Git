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
int sockfd;


//CONFIGURE//////////////////////////////////////////////////////////////
/**
writes .configure file in directory of executable with IP and PORT info
**/
void writeConfigureFile(char* IP, char* port){
	//CHECK ARGUMENTS
	//checking IP
		if( gethostbyname(IP) == NULL ) pEXIT_ERROR("issue reading IP");
	//checking port
		int pnum = (int)strtol(port, NULL, 10);
		if(pnum<8000 ||pnum>65535) pEXIT_ERROR("port must be an int between 8000 and 65535");

	//CREATE CONFIGURE FILE
	int file = openFileW("./.configure");
		if(file<0) pEXIT_ERROR("writing configure file");

	//WRITE IP and Port info to file
	WRITE_AND_CHECKv(file, IP, strlen(IP));
	WRITE_AND_CHECKv(file, "\n", 1);
	WRITE_AND_CHECKv(file, port, strlen(port));

	close(file);
	return;
}


/**
initializes IP and PORT by reading the configure file
**/
void initializeIPandPort(char** IP_addr, int* PORT_addr){
	char* conf = readFile("./.configure");
		if(conf==NULL) pEXIT_ERROR("error reading file, did not configure");

	//SET IP AND PORT
	*IP_addr = strtok(conf, "\n");
	*PORT_addr = (int)strtol(strtok(NULL, "\n"), NULL, 10);

	return;
}
/////////////////////////////////////////////////////////////////////////


//[3.1] CHECKOUT//////////////////////////////////////////////////////////
void checkoutClient(char* proj_name){ //TODO
	int num_bytes = strlen("checkout") + 1 + strlen(proj_name);

	//sending over num_bytes
	WRITE_AND_CHECKv(sockfd, &num_bytes,  4);

	//sending over all information
	WRITE_AND_CHECKv(sockfd, "checkout", strlen("checkout"));
	WRITE_AND_CHECKv(sockfd, ":" , 1);
	WRITE_AND_CHECKv(sockfd, proj_name , strlen(proj_name));

	return;
}
////////////////////////////////////////////////////////////////////////



//[3.2] UPDATE//////////////////////////////////////////////////////////////
void updateClient(char* proj_name){
	return;
}
///////////////////////////////////////////////////////////////////////////



//[3.3] UPGRADE//////////////////////////////////////////////////////////////
void upgradeClient(char* proj_name){
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.4] COMMIT//////////////////////////////////////////////////////////////
void commitClient(char* proj_name){
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.5] PUSH//////////////////////////////////////////////////////////////
void pushClient(char* proj_name){
	return;
}
/////////////////////////////////////////////////////////////////////////



//[3.6] CREATE//////////////////////////////////////////////////////////////
void createClient(char* proj_name){
	//printf("%d\n", sockfd); //TODO
	printf("entered create client\n");
	//sending over number of bytes
	int num_bytes = strlen("create") + 1 + strlen(proj_name);

	//sending over num_bytes
	WRITE_AND_CHECKv(sockfd, &num_bytes,  4);

	//sending over all information
	WRITE_AND_CHECKv(sockfd, "create", strlen("create"));
	WRITE_AND_CHECKv(sockfd, ":" , 1);
	WRITE_AND_CHECKv(sockfd, proj_name , strlen(proj_name));

	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.7] DESTROY//////////////////////////////////////////////////////////////
void destroyClient(char* proj_name){
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.8] ADD//////////////////////////////////////////////////////////////
void addClient(char* proj_name, char* file_name){
	return;
}
////////////////////////////////////////////////////////////////////////



//[3.9] REMOVE//////////////////////////////////////////////////////////////
void removeClient(char* proj_name, char* file_name){
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.10] CURRENT VERSION/////////////////////////////////////////////////////
void currentVersionClient(char* proj_name){
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.11] HISTORY//////////////////////////////////////////////////////////////
void historyClient(char* proj_name){
	return;
}
//////////////////////////////////////////////////////////////////////////////



//[3.12] ROLLBACK//////////////////////////////////////////////////////////////
void rollbackClient(char* proj_name){
	return;
}
//////////////////////////////////////////////////////////////////////////////




//CONNECT//////////////////////////////////////////////////////////////

/**
connects to server using the "server.configure" file  TODO print out statements
**/
void connectToServer(){
	//initialize IP and PORT_addr
		char* IP;
		int PORT;
		initializeIPandPort(&IP, &PORT);

	//declaring vars
		//struct sockaddr_in sock_addr;
		struct sockaddr_in serv_addr;

	//get server address
		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(PORT); //convert to byte addr
		if(inet_pton(AF_INET, IP , &serv_addr.sin_addr)<=0) //convert to byte addr
			pEXIT_ERROR("Server Address");

	//connect to server
		if( (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0 )
			pEXIT_ERROR("Connection to Server Failure");

	printf("Successfully connected to server!\n");
	return;
}
////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv){
	if(argc<2) pEXIT_ERROR("Not enough arguments");

	char* command = argv[1];

	//initialize socket
	if( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		pEXIT_ERROR("Socket Creation");

	//MAKE CONFIGURE FILE
	if(strcmp(command,"configure")==0){
		if(argc!=4) pEXIT_ERROR("configure must be followed by 2 arguments: IP and Port");
		//write config file and return
		writeConfigureFile(argv[2], argv[3]);
		return 0;
	}

	//CONNECT TO SERVER
		connectToServer();

	//COMMANDS
	//[3.1] checkout
	if (strcmp(command,"checkout")==0){
		if(argc!=3) pEXIT_ERROR("checkout must be followed by 1 argument: project name");
		checkoutClient(argv[2]);

	//[3.2] update
	}else if (strcmp(command,"update")==0){
		if(argc!=3) pEXIT_ERROR("update must be followed by 1 argument: project name");
		updateClient(argv[2]);

	//[3.3] upgrade
	}else if (strcmp(command,"upgrade")==0){
		if(argc!=3) pEXIT_ERROR("upgrade must be followed by 1 argument: project name");
		upgradeClient(argv[2]);

	//[3.4] commit
	}else if (strcmp(command,"commit")==0){
		if(argc!=3) pEXIT_ERROR("commit must be followed by 1 argument: project name");
		commitClient(argv[2]);

	//[3.5] push
	}else if (strcmp(command,"push")==0){
		if(argc!=3) pEXIT_ERROR("push must be followed by 1 argument: project name");
		pushClient(argv[2]);

	//[3.6] create
	}else if (strcmp(command,"create")==0){
		if(argc!=3) pEXIT_ERROR("create must be followed by 1 argument: project name");
		createClient(argv[2]);

	//[3.7] destroy
	}else if (strcmp(command,"destroy")==0){
		if(argc!=3) pEXIT_ERROR("destroy must be followed by 1 argument: project name");
		destroyClient(argv[2]);

	//[3.8] add
	}else if (strcmp(command,"add")==0){
		if(argc!=4) pEXIT_ERROR("destroy must be followed by 2 arguments: project name and file name");
		addClient(argv[2], argv[3]);

	//[3.9] remove
	}else if (strcmp(command,"remove")==0){
		if(argc!=4) pEXIT_ERROR("remove must be followed by 2 arguments: project name and file name");
		removeClient(argv[2], argv[3]);

	//[3.10] currentversion
	}else if (strcmp(command,"currentversion")==0){
		if(argc!=3) pEXIT_ERROR("currentversion must be followed by 1 argument: project name");
		currentVersionClient(argv[2]);

	//[3.11] history
	}else if (strcmp(command,"history")==0){
		if(argc!=3) pEXIT_ERROR("history must be followed by 1 argument: project name");
		historyClient(argv[2]);

	//[3.12] rollback
	}else if (strcmp(command,"rollback")==0){
		if(argc!=4) pEXIT_ERROR("rollback must be followed by 2 arguments: project name, rollback");
		rollbackClient(argv[2]);

	}else{
		pEXIT_ERROR("did not enter in a valid command (refrence ReadMe for valid commands)");
	}



	return 0;
}
