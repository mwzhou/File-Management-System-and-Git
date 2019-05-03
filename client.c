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

#include "client.h"

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
	WRITE_AND_CHECKe(file, IP, strlen(IP));
	WRITE_AND_CHECKe(file, "\n", 1);
	WRITE_AND_CHECKe(file, port, strlen(port));

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
	printf("%d] Entered command: checkout\n", sockfd);
	sendArgsToServer("checkout", proj_name, NULL);

	/*ERROR CHECK*/
		//waiting if proj does not exist on server
		if( receiveSig(sockfd) == false ) pEXIT_ERROR("project does not exist on Server");
		//check if file already exists on Client
		if( sendSig( sockfd, ( typeOfFile(proj_name)== isDIR ) ) == false ) pEXIT_ERROR("project name already exists on Client side");



	/*recieving project and untaring it in root*/
		char* proj_tar = recieveTarFile( sockfd, "./");
			if(proj_tar == NULL){ pEXIT_ERROR("error recieving directory"); }

	/*make backup directory*/
		char* backup_proj_dir = concatString(proj_name, ".bak");
		if( mkdir( backup_proj_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){ pEXIT_ERROR("mkdir()"); }

	free(proj_tar);
	free(backup_proj_dir);
	return;
}
////////////////////////////////////////////////////////////////////////


//[3.2] UPDATE//////////////////////////////////////////////////////////////
void updateClient(char* proj_name){
	printf("%d] Entered command: update\n", sockfd);

	sendArgsToServer("update", proj_name, NULL);

	/**ERROR CHECK**/
		//check if project name doesn't exist on Server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project does not exist on Server");
		//check if manifest doesn't exist on Server
		if( receiveSig(sockfd) == false ) pEXIT_ERROR(".Manifest file does not exist on Server");


	/*recieve manifest files*/
		//recieving manifest file from server
			char* store_manifest_serv_dir = concatString(proj_name, ".bak");  //make path to store manifest file recieved from Server

			char* manifest_serv_path = recieveTarFile(sockfd, store_manifest_serv_dir);
				free(store_manifest_serv_dir);
				if( manifest_serv_path == NULL ) pEXIT_ERROR("recieving Manifest File");

		//creating manifest file for local directory
			char* manifest_client_path = combinedPath( proj_name, ".Manifest" );
			//TODO: hash? or nah

	/*OPERATIONS*/
		//get client and server manifest
		char* client_manifest = readFile( manifest_client_path );
			free(manifest_client_path);
			if(  client_manifest ) pEXIT_ERROR("read");
		char* server_manifest = readFile( manifest_serv_path );
			free(manifest_serv_path);
			if( server_manifest ) pEXIT_ERROR("read");


		//create update file
			//get path for .Update file
			char* update_path = combinedPath( proj_name, ".Update" );
			//create file
			int update_fd = openFileW( update_path );
				free( update_path );
				if( update_fd < 0 ) pEXIT_ERROR("update");


	/*Comparisons*/ //TODO AVL TREE
			printf("%s\n%s\n%d\n", client_manifest, server_manifest, update_fd); //TODO

	//free and return
	free( client_manifest );
	free( server_manifest );
	return;
}
///////////////////////////////////////////////////////////////////////////


//[3.3] UPGRADE//////////////////////////////////////////////////////////////
void upgradeClient(char* proj_name){
	printf("%d] Entered command: upgrade\n", sockfd);

	sendArgsToServer("upgrade", proj_name, NULL);

	/*ERROR CHECK*/
		//waiting for signal if valid project on server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project doesn't exist on server");
		//check if update path exists on Client
		char* update_path = combinedPath(proj_name, ".Update");
		if( sendSig( sockfd, (typeOfFile(update_path)!=isREG )) == false ){ free(update_path); pEXIT_ERROR(".Update file doesn't exist on Client"); }


	/*OPERATIONS*/

	//recieving...

	free(update_path);
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.4] COMMIT//////////////////////////////////////////////////////////////
void commitClient(char* proj_name){
	printf("%d] Entered command: commit\n", sockfd);

	//sending arguments to server
		sendArgsToServer("commit", proj_name, NULL);

	/*ERROR CHECK*/
		//waiting for signal if valid project on server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project doesn't exist on server");
		//check if update file exists and is NOT empty
		char* update_path = combinedPath(proj_name, ".Update");
		if( sendSig( sockfd, ( typeOfFile(update_path)!=isUNDEF && sizeOfFile(update_path)!=0 ) ) == false ){ free(update_path); pEXIT_ERROR(".Update file is nonempty on Client!"); } //TODO

	/*OPERATIONS*/

	free(update_path);
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.5] PUSH//////////////////////////////////////////////////////////////
void pushClient(char* proj_name){
	printf("%d] Entered command: push\n", sockfd);
	sendArgsToServer("push", proj_name, NULL);

	return;
}
/////////////////////////////////////////////////////////////////////////



//[3.6] CREATE//////////////////////////////////////////////////////////////
void createClient(char* proj_name){
	printf("%d] Entered command: create\n", sockfd);
	sendArgsToServer("create", proj_name, NULL);

	/*make directory*/
	if( mkdir( proj_name , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){ pEXIT_ERROR("mkdir()"); }

	/*make backup directory*/
	char* backup_proj_dir = concatString(proj_name, ".bak");
	if( mkdir( backup_proj_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){free(backup_proj_dir); pEXIT_ERROR("mkdir()"); }
	free(backup_proj_dir);

	/*recieving manifest file*/
	char* manifest_path = recieveTarFile( sockfd, proj_name );
		if(manifest_path == NULL){ pEXIT_ERROR("recieving .Manifest file from Server"); }

	//TODO

	//free and return
	printf("\n\tSuccessfully created project: %s on Client side!\n", proj_name);
	free(manifest_path);
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
	printf("%d] Entered command: add\n", sockfd);

	//check if project exists on client side or not
	if( typeOfFile(proj_name)!=isDIR ){ pEXIT_ERROR("Project does not exist in client!"); }

	//Get paths of manifest file and file to write into manifest file
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	int manifest_fd = open( manifest_path, O_WRONLY|O_APPEND, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) ); //writing in manifest file
		if(manifest_fd < 0){ fprintf( stderr, "file:%s\n",file_name ); pRETURN_ERRORvoid("tried to open file flags: (O_WRONLY|O_APPEND)"); }

	if(manifest_fd<0) { pRETURN_ERRORvoid("Error on opening file"); }

	//find path of file and generate hashcode for it
	char* new_path = combinedPath(proj_name, file_name);
	char* hash_code = generateHash(new_path);

	//Write info into manifet file for new file that is added
	WRITE_AND_CHECKv( manifest_fd, new_path, strlen(new_path));
	WRITE_AND_CHECKv( manifest_fd, "\t", 1);
	WRITE_AND_CHECKv( manifest_fd, "1", 1);
	WRITE_AND_CHECKv( manifest_fd, "\t", 1);
	WRITE_AND_CHECKv( manifest_fd, hash_code, strlen(hash_code));
	WRITE_AND_CHECKv( manifest_fd, "\n", 1);

	//freeing and closing
	free(hash_code);
	free(new_path);
	close(manifest_fd);
	free(manifest_path);

	return;
}
////////////////////////////////////////////////////////////////////////



//[3.9] REMOVE//////////////////////////////////////////////////////////////
void removeClient(char* proj_name, char* file_name){

	printf("%d] Entered command: remove\n", sockfd);

	//check valid arguments
	if ( typeOfFile(proj_name)!= isDIR ){ pEXIT_ERROR("project name does not exist on client side"); }

		//Finding path of manifest file
	char* manifest_path = combinedPath(proj_name, ".Manifest");

	//finding relative path of file_name
	char* file_path = combinedPath(proj_name, file_name);

	//line to delete
	int line = extractLine(manifest_path, file_path);

	//Files to read and rewrite in
	FILE* srcFile = fopen(manifest_path, "r");
	FILE* tempFile = fopen("deleteFile.tmp","w");

	//inserting into tempFile not include line to delete
	int lineSize = 1024;
	char buffer[lineSize];
	int count = 1;
	while((fgets(buffer, lineSize, srcFile) )!=NULL){
		if(line!=count)
			fputs(buffer,tempFile);
		count++;
	}

	//replace old file with new file
	remove(manifest_path);
	rename("deleteFile.tmp",manifest_path);

	//free and close
	free(manifest_path);
	free(file_path);
	fclose(srcFile);
	fclose(tempFile);

	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.10] CURRENT VERSION/////////////////////////////////////////////////////
void currentVersionClient(char* proj_name){
	printf("%d] Entered command: currentversion\n", sockfd);
	sendArgsToServer("currentversion", proj_name, NULL);

	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.11] HISTORY//////////////////////////////////////////////////////////////
void historyClient(char* proj_name){
	printf("%d] Entered command: history\n", sockfd);
	sendArgsToServer("history", proj_name, NULL);

	return;
}
//////////////////////////////////////////////////////////////////////////////



//[3.12] ROLLBACK//////////////////////////////////////////////////////////////
void rollbackClient(char* proj_name, char* version){
	printf("%d] Entered command: rollback\n", sockfd);
	sendArgsToServer("rollback", proj_name, version);

	/*ERROR check*/
	//check version number
	int v_num = atoi(version);
	if(  sendSig( sockfd, (v_num <= 0) ) == false ) pEXIT_ERROR("invalid version number");



	return;
}
//////////////////////////////////////////////////////////////////////////////



//MISC////////////////////////////////////////////////////////////////////////////
/**
writes arguments to server
**/
void sendArgsToServer(char* s1, char* s2, char* s3){


	//sending over number of bytes
	printf("\tSending over num_bytes of info to server\n");
	long num_bytes = (s3==NULL)? (strlen(s1) + 1 + strlen(s2))  :  (strlen(s1) + 1 + strlen(s2) + 1 + strlen(s3)) ;

	WRITE_AND_CHECKe(sockfd, &num_bytes,  4);

	//sending over all information
	printf("\tSending over information to server\n");
	char delim = (char)176;
	WRITE_AND_CHECKe(sockfd, s1 , strlen(s1));
	WRITE_AND_CHECKe(sockfd, &delim , 1);
	WRITE_AND_CHECKe(sockfd, s2 , strlen(s2));

	if(s3!=NULL){
		WRITE_AND_CHECKe(sockfd, &delim  , 1);
		WRITE_AND_CHECKe(sockfd, s3 , strlen(s3));
	}

}


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
		printf("configured successfully\n");
		return 0;
	}

	//CONNECT TO SERVER
		connectToServer();


	//COMMANDS
	//[3.1] checkout
	if (strcmp(command,"checkout")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("checkout must be followed by 1 argument: project name"); }
		checkoutClient(argv[2]);

	//[3.2] update
	}else if (strcmp(command,"update")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("update must be followed by 1 argument: project name"); }
		updateClient(argv[2]);

	//[3.3] upgrade
	}else if (strcmp(command,"upgrade")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("upgrade must be followed by 1 argument: project name"); }
		upgradeClient(argv[2]);

	//[3.4] commit
	}else if (strcmp(command,"commit")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("commit must be followed by 1 argument: project name"); }
		commitClient(argv[2]);

	//[3.5] push
	}else if (strcmp(command,"push")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("push must be followed by 1 argument: project name"); }
		pushClient(argv[2]);

	//[3.6] create
	}else if (strcmp(command,"create")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("create must be followed by 1 argument: project name"); }
		createClient(argv[2]);

	//[3.7] destroy
	}else if (strcmp(command,"destroy")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("destroy must be followed by 1 argument: project name"); }
		destroyClient(argv[2]);

	//[3.8] add
	}else if (strcmp(command,"add")==0){
		if(argc!=4) { sendErrorSocket(sockfd); pEXIT_ERROR("destroy must be followed by 2 arguments: project name and file name"); }
		addClient(argv[2], argv[3]);

	//[3.9] remove
	}else if (strcmp(command,"remove")==0){
		if(argc!=4) { sendErrorSocket(sockfd); pEXIT_ERROR("remove must be followed by 2 arguments: project name and file name"); }
		removeClient(argv[2], argv[3]);

	//[3.10] currentversion
	}else if (strcmp(command,"currentversion")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("currentversion must be followed by 1 argument: project name"); }
		currentVersionClient(argv[2]);

	//[3.11] history
	}else if (strcmp(command,"history")==0){
		if(argc!=3) { sendErrorSocket(sockfd); pEXIT_ERROR("history must be followed by 1 argument: project name"); }
		historyClient(argv[2]);

	//[3.12] rollback
	}else if (strcmp(command,"rollback")==0){
		if(argc!=4) { sendErrorSocket(sockfd); pEXIT_ERROR("rollback must be followed by 2 arguments: project name, rollback"); }
		rollbackClient(argv[2], argv[3]);

	}else{
		sendErrorSocket(sockfd); pEXIT_ERROR("did not enter in a valid command (refrence ReadMe for valid commands)");
	}



	return 0;
}
