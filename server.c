#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#include "server.h"


//GLOBALS////////////////////////////////////////////////////////////////
#define BACKLOG 50
int overall_socket;

ClientThread clients[BACKLOG];
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;

ProjectNode *head = NULL;
//[3.1] CHECKOUT////////////////////////////////////////////////////////////////////////

void checkoutServer( int sockfd, char* proj_name ){
	printf("\nEntered command: checkout\n");

	/*ERROR CHECK*/
		//check if proj exists on Server - send message to client
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERRORvoid("project doesn't exist on Server");
		//waiting for Client to see if project exists
		if( receiveSig( sockfd ) == false ) pRETURN_ERRORvoid("Project already exists on Client");


	/**SEND project over to client**/
		//get backup folder_dir
		char* bakup_proj_path = concatString( proj_name, ".bak" );

		if ( sendTarFile(sockfd, proj_name, bakup_proj_path) == false){ pRETURN_ERRORvoid("error sending .Manifest file"); }
			free(bakup_proj_path);
}
////////////////////////////////////////////////////////////////////////


//[3.2] UPDATE//////////////////////////////////////////////////////////////////////


/*
update
*/
void updateServer(  int sockfd, char* proj_name  ){
	printf("\nEntered command: update\n");

	/*ERROR CHECK*/
		//check if project name doesn't exist on Server
		if( sendSig(sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false ) pRETURN_ERRORvoid("project doesn't exist on server");
		//check if manifest doesn't on Server
		char* manifest_path = combinedPath( proj_name, ".Manifest"); //get path of manifest
		if( sendSig(sockfd, ( typeOfFile(manifest_path) != isREG ) ) == false ){ free(manifest_path);  pRETURN_ERRORvoid(".Manifest file doesn't exist in project on server"); }



	/*SEND manifest file to client*/
		char* bakup_proj = concatString( proj_name, ".bak" );
		//send
		if ( sendTarFile(sockfd, manifest_path, bakup_proj) == false){ free(manifest_path); free(bakup_proj); pRETURN_ERRORvoid("error sending .Manifest file"); }
			free(manifest_path);
			free(bakup_proj);
}
////////////////////////////////////////////////////////////////////////


//[3.3] UPGRADE//////////////////////////////////////////////////////////////
void upgradeServer(  int sockfd, char* proj_name  ){
	printf("\nEntered command: upgrade\n");

	/*ERROR CHECK*/
		//check if project doesn't exist on Server
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERRORvoid("project doesn't exist on server");
		//check if .Update exists on Client
		if( receiveSig(sockfd) == false){  printf("\tPlease update first (no update file on client)\n"); return;  }
		//check if .Update file is emptry
		if( receiveSig(sockfd) == false){ printf("\tProject is up to date\n"); return; }


	/*OPERATIONS*/
	/**SEND project over to client**/
		char* backup_proj_path = concatString( proj_name, ".bak" ); //get backup folder_dir
		if ( sendTarFile(sockfd, proj_name, backup_proj_path) == false ){ pRETURN_ERRORvoid("error sending .Manifest file"); }
   	free(backup_proj_path);
}
////////////////////////////////////////////////////////////////////////


//[3.4] COMMIT//////////////////////////////////////////////////////////////
void commitServer( int sockfd, char* proj_name ){
	printf("\n\tEntered command: commit\n");

	/*ERROR CHECK*/
		//check if project exists on Server
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERRORvoid("project doesn't exist on server");
		//wait from client if update file is empty or doesn't exist
		if( receiveSig(sockfd) == false) pRETURN_ERRORvoid(".Update file is nonempty on Client!");

	//sending project .Manifest to client
	char* bakup_proj = concatString( proj_name, ".bak" );
	char* manifest_path = combinedPath(proj_name,".Manifest");
	if (sendTarFile( sockfd, manifest_path, bakup_proj) == false) {pRETURN_ERRORvoid("sendTarFile failed");}

	//freeing
	free(bakup_proj);
	free(manifest_path);

	return ;
}
////////////////////////////////////////////////////////////////////////



//[3.5] PUSH//////////////////////////////////////////////////////////////
void pushServer(  int sockfd, char* proj_name  ){
	printf("\nEntered command: push\n");

	/*ERROR CHECK*/
		//check if project name doesn't exist on Server
		if( sendSig(sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false ) pRETURN_ERRORvoid("project doesn't exist on server");
		//check if manifest doesn't on Server
		char* commit_server_path = combinedPath( proj_name, ".Commit"); //get path of manifest
		if( sendSig(sockfd, ( typeOfFile(commit_server_path) != isREG ) ) == false ){ free(commit_server_path);  pRETURN_ERRORvoid(".Commit file doesn't exist in project on server"); }
		//if file was modified on client side
		if( receiveSig(sockfd) == false ) {free(commit_server_path); pRETURN_ERRORvoid("A file was modified since the last upgrade on the client side");}


	//TODO lock the repository

	//recieving files send from client	
	char* bakup_proj = concatString( proj_name, ".bak" );
	char* dir_of_files = recieveTarFile( sockfd, bakup_proj );
		if(dir_of_files == NULL){ pRETURN_ERRORvoid("recieveTarFile failed"); }

	//get client commit file:
	char* commit_client_path = combinedPath(dir_of_files,".Commit");

	//ERROR CHECK: if client Commit does not match server commit
	char* commitServer = readFile(commit_server_path);
	char* commitClient = readFile(commit_client_path);
	if( sendSig(sockfd, ( strcmp(commitClient, commitServer)!=0 ) ) == false ){ 
		//delete files sent to server from client
		bool delete_client_files = removeDir(dir_of_files);
		if(delete_client_files==false){pRETURN_ERRORvoid("Removing directory failed");}
		free(commitClient); 
		free(commit_server_path); 
		free(bakup_proj); 
		free(dir_of_files); 
		free(commitServer); 
		pRETURN_ERRORvoid(".Commit of client and server are different!");
	}
	free(commitClient); 
	free(commitServer);
	//TODO expire All other commits from other cliets if commit files are the same

	//Get server's current project version to copy and rename project.
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	FILE* mP = fopen(manifest_path,"r");
	int count = 0;
	char* og_pNum;
	int lineSize = 1024;
	char buffer[lineSize];

	//get project Number
	while(count<2){
		fgets(buffer, lineSize, mP);
		count++;
		if(count==2){
			og_pNum = substr(buffer,0,2);
		}
	}
	//go back to beginning of file and close
	rewind(mP);
	fclose(mP);

	//creat bakup version
	char* copyPath = combinedPath(bakup_proj,og_pNum);

	//copy backup version into into directory
	bool s = copyDir(proj_name, copyPath);
	if(s==false){pRETURN_ERRORvoid("copying failed");}

	//tar version file and delete normal file
	makeTar(copyPath, bakup_proj);
	bool delete_version_file = removeDir(copyPath);
	if(delete_version_file==false){pRETURN_ERRORvoid("Removing directory failed");}

	//TODO UMAD

	//incrementing project number and replacing server's manifest with client's manifest
	char* manifest_client_path = combinedPath(dir_of_files,".Manifest");
	char* temp_path = combinedPath(dir_of_files,"replace.tmp");
	FILE* tempFile = fopen(temp_path,"w");
	FILE* cmP = fopen(manifest_client_path, "r");
	int line = 0;
	while((fgets(buffer, lineSize, cmP) )!=NULL)
        {
		line++;
		if(line==2){
			char* num = substr(buffer, 0, 2);
			int vNum = atoi(num);
			free(num);
			vNum++;
			int len = (int)((ceil(log10(vNum))+1)*sizeof(char));
			char str[len];
			sprintf(str, "%d", vNum);
			fputs(str, tempFile);
			fputs("\n", tempFile);
		}
		else{
			fputs(buffer, tempFile);
		}
        }
	fputs("\n", tempFile);
	//replace manifest with new one
	remove(manifest_client_path);
	rename(temp_path,manifest_client_path);
	fclose(cmP);
	fclose(tempFile);

	//moving new manifest to clients's project and replacing it
	moveFile(manifest_client_path, proj_name);

	//Removing and Freeing
	bool delete_client_files = removeDir(dir_of_files);
	if(delete_client_files==false){pRETURN_ERRORvoid("Removing directory failed");}
	free(manifest_path);
	free(temp_path);
	free(og_pNum);
	free(commit_client_path);
	free(commit_server_path);
	free(manifest_client_path);
	free(copyPath);
	free(bakup_proj);
}
////////////////////////////////////////////////////////////////////////


//[3.6] CREATE//////////////////////////////////////////////////////////////
void createServer(  int sockfd, char* proj_name ){
	printf("\nEntered command: create\n");
	/*error check*/
	if( typeOfFile(proj_name)==isDIR ){ sendErrorSocket(sockfd); pRETURN_ERRORvoid("project already exists on server"); }


	/*make directory*/
	if( mkdir( proj_name , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){ pRETURN_ERRORvoid("mkdir()"); }


	/*make .Manifest File*/
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	int manifest_fd = openFileW( manifest_path );
		if( manifest_fd < 0){ free(manifest_path); pRETURN_ERRORvoid("open"); }
	WRITE_AND_CHECKv(manifest_fd, "1\n", 2);
	WRITE_AND_CHECKv(manifest_fd, "1\n", 2);

	/*make backup directory*/
	char* backup_proj_dir = concatString(proj_name, ".bak");
	if( mkdir( backup_proj_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){ free(manifest_path); pRETURN_ERRORvoid("mkdir()"); }

	/*send Manifest file to client*/
	if( sendTarFile( sockfd, manifest_path, backup_proj_dir) == false){ free(backup_proj_dir); free(manifest_path); pRETURN_ERRORvoid("sending .Manifest file to client"); }

	/*Add project to global linked list*/
	addProjectNode( proj_name );

	//free and return
	printf("\nSuccessfully created project: %s on Server side!\n", proj_name);

	free(backup_proj_dir);
	free(manifest_path);
	close(manifest_fd);

	return;
}
////////////////////////////////////////////////////////////////////////


//DESTROY//////////////////////////////////////////////////////////////////////
void destroyServer(int sockfd, char* proj_name ){
	if( removeDir( proj_name ) == false){ pRETURN_ERRORvoid("remove"); } //TODO : error check and signals
	return;

}
////////////////////////////////////////////////////////////////////////


//CURRVERSION//////////////////////////////////////////////////////////////////////
void currentversionServer(  int sockfd, char* proj_name  ){
	/*ERROR CHECK*/
		//check if project exists on Server
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERRORvoid("project doesn't exist on server");
		//check if manifest doesn't on Server
		char* manifest_path = combinedPath( proj_name, ".Manifest"); //get path of manifest
		if( sendSig(sockfd, ( typeOfFile(manifest_path) != isREG ) ) == false ){ free(manifest_path);  pRETURN_ERRORvoid(".Manifest file doesn't exist in project on server"); }


	/*SEND manifest file to client*/
			char* manifest_str = readFile( manifest_path);
				free(manifest_path);
				if(manifest_str==NULL){ pRETURN_ERRORvoid("reading manifest"); }
			//send manifest_str
			 if( sendStringSocket( sockfd, manifest_str) == false){ pRETURN_ERRORvoid("sending manifest string"); }

	free(manifest_str);

}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void historyServer( int sockfd, char* proj_name  ){ //TODO Client

	/*ERROR CHECK*/
		//check if project exists on Server
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERRORvoid("project doesn't exist on server");

	return;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void rollbackServer(  int sockfd, char* proj_name, char* version_num){ //TODO Client

	/*ERROR CHECK*/
		//check if project exists on Server
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERRORvoid("project doesn't exist on server");
		//wait from client if update file is empty or doesn't exist
		if( receiveSig(sockfd) == false) pRETURN_ERRORvoid("Version Number is invalid!");


	return;
}
////////////////////////////////////////////////////////////////////////



//CONNECT//////////////////////////////////////////////////////////////////////
//Handles accepting information sent in by the client
void* connect_client( void* curr_clientthread ){
	ClientThread* args = (ClientThread*)curr_clientthread;
	int sockfd = args->curr_socket;
  
	printf("%d] Success on connection!\n", sockfd);

	/*Recieve arguments from Client*/
		char* arguments = recieveStringSocket( sockfd );
			if( arguments == NULL ){ pRETURN_ERROR("recieving arguments", NULL); }
	//Parse through info and store in variables
		char delim[2]; delim[0] = (char)176; delim[1] = '\0';
		char* command = copyString( strtok(arguments, delim) );
		char* proj_name = copyString( strtok(NULL, delim) );
		char* s3 = strtok(NULL, delim); //file or version_num if applicable
			if( s3 != NULL ) s3 = copyString( s3 );

		if(s3==NULL) printf("\tRecieved from client - command:%s  proj_name:%s \n", command, proj_name);
		else printf("\tRecieved from client - command:%s  proj_name:%s  version_num:%s \n", command, proj_name, s3);

		free( arguments );



	//The following if statements call methods based on the request sent from the client
	if(strcmp(command,"checkout")==0)
		checkoutServer(sockfd, proj_name);

	else if(strcmp(command,"update")==0)
		updateServer(sockfd, proj_name);

	else if(strcmp(command,"upgrade")==0)
		upgradeServer(sockfd, proj_name);

	else if(strcmp(command,"commit")==0)
		commitServer(sockfd, proj_name);

	else if(strcmp(command,"push")==0)
		pushServer(sockfd, proj_name);

	else if(strcmp(command,"create")==0)
		createServer(sockfd, proj_name);

	else if(strcmp(command,"destroy")==0)
		destroyServer(sockfd, proj_name);

	else if(strcmp(command,"currentversion")==0)
		currentversionServer(sockfd, proj_name);

	else if(strcmp(command,"history")==0)
		historyServer(sockfd, proj_name);

	else if(strcmp(command,"rollback")==0)
		rollbackServer(sockfd, proj_name, s3);

	else

		printf("\tError on client side argument\n");

	/*EXITING CLIENT*/
	shutdown(sockfd , SHUT_RDWR );
	printf("[closing client sock: %d]\n", sockfd);

	//aquiring mutex
	pthread_mutex_lock( &clients_lock );

	if(close(sockfd) < 0) pRETURN_ERROR("Error on Close", NULL);
	args->done = true;

	//releasing mutex
	pthread_mutex_unlock( &clients_lock );

	pthread_exit(NULL);

}
////////////////////////////////////////////////////////////////////////



int main(int argc, char * argv[]){
	//Check for arguments
		if(argc!=2) pRETURN_ERROR("Enter an argument containing the port number\n",-1);

	/*Initialize Clients Global Array*/
		int i;
		for( i = 0; i<BACKLOG ; i++ ){
			ClientThread curr = {-1, -1, true};
			clients[i] = curr;
		}

	//getting port
		int port = (int)atol(argv[1]);

	//server address
		struct sockaddr_in address;

	//CREATING SOCKET
	overall_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(overall_socket == 0) pRETURN_ERROR("Error on socket creation",-1);
	//reuse socket
		if (setsockopt(overall_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)  pRETURN_ERROR("setsockopt(SO_REUSEADDR) failed",-1);

	//intitializes sizeof(address) zero-value bytes of address
	bzero(&address,sizeof(address));

	//initializes sockaddr_in functions for address
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	//BINDING to client
	int addrlen = sizeof(address);
	int status = bind(overall_socket, (struct sockaddr*) &address, addrlen);
		if(status < 0) pRETURN_ERROR("Error on Bind",-1);

	//LISTENto client
	status = listen(overall_socket, 20);
		if(status < 0) pRETURN_ERROR("Error on Listen",-1);


	//ACCEPT connecting and accepting message for client
	int curr_socket;
	while( (curr_socket= accept(overall_socket, (struct sockaddr*) &address, (socklen_t*)&addrlen)) > 0 ){

		//find first open socket
		for(i=0; i<BACKLOG; i++){
			if( clients[i].done == true){ //if found empty
				clients[i].curr_socket = curr_socket;
				clients[i].done = false;
				break;
			}
		}

		//connects to client through thread!
		if( pthread_create(&clients[i].client , NULL, connect_client, (void*)(&clients[i]) ) < 0 ) pRETURN_ERROR("Thread not created",-1);
	}

	//if accept failed
	if(curr_socket<0){ pRETURN_ERROR("Connection to client failed",-1); }

	if(close(overall_socket) < 0) pRETURN_ERROR("Error on Close",-1);
	return 0;	//initialize overall_socket
}
