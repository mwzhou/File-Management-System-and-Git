#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

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
int overall_socket;

ClientThread clients[20];
int num_clients = 0;

ProjectNode *head = NULL;

//[3.1] CHECKOUT////////////////////////////////////////////////////////////////////////

void* checkoutServer( int sockfd, char* proj_name ){
	printf("\n\tEntered command: checkout\n");

	/*ERROR CHECK*/
		//check if proj exists on Server - send message to client
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERROR("project doesn't exist on Server",NULL);
		//waiting for Client to see if project exists
		if( receiveSig( sockfd ) == false ) pRETURN_ERROR("Project already exists on Client",NULL);

	/**SEND project over to client**/
		//get backup folder_dir
		char* bakup_proj_path = concatString( proj_name, ".bak" );
		if ( sendTarFile(sockfd, proj_name, bakup_proj_path) == false){ pRETURN_ERROR("error sending .Manifest file", NULL); }
			free(bakup_proj_path);

	return 0;
}
////////////////////////////////////////////////////////////////////////


//[3.2] UPDATE//////////////////////////////////////////////////////////////////////


/*
update
*/
void* updateServer(  int sockfd, char* proj_name  ){
	printf("\n\tEntered command: update\n");

	/*ERROR CHECK*/
		//check if project name doesn't exist on Server
		if( sendSig(sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false ) pRETURN_ERROR("project doesn't exist on server",NULL);
		//check if manifest doesn't on Server
		char* manifest_path = combinedPath( proj_name, ".Manifest"); //get path of manifest
		if( sendSig(sockfd, ( typeOfFile(manifest_path) != isREG ) ) == false ){ free(manifest_path);  pRETURN_ERROR(".Manifest file doesn't exist in project on server",NULL); }


	/*SEND manifest file to client*/
		char* bakup_proj = concatString( proj_name, ".bak" );
		//send
		if ( sendTarFile(sockfd, manifest_path, bakup_proj) == false){ pRETURN_ERROR("error sending .Manifest file", NULL); }
			free(manifest_path);
			free(bakup_proj);


		//TODO: operations



	return 0;
}
////////////////////////////////////////////////////////////////////////


//[3.3] UPGRADE//////////////////////////////////////////////////////////////
void* upgradeServer(  int sockfd, char* proj_name  ){
		printf("\n\tEntered command: upgrade\n");

		/*ERROR CHECK*/
			//check if project exists on Server
			if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERROR("project doesn't exist on server",NULL);
			//check if .Update exists on Client
			if( receiveSig(sockfd) == false) pRETURN_ERROR(".Update doesn't exist on Client",NULL);

	return 0;
}
////////////////////////////////////////////////////////////////////////


//[3.4] COMMIT//////////////////////////////////////////////////////////////
void* commitServer( int sockfd, char* proj_name ){
	printf("\n\tEntered command: commit\n");

	/*ERROR CHECK*/
		//check if project exists on Server
		if( sendSig( sockfd, ( typeOfFile(proj_name)!=isDIR ) ) == false) pRETURN_ERROR("project doesn't exist on server",NULL);
		//wait from client if update file is empty or doesn't exist
		if( receiveSig(sockfd) == false) pEXIT_ERROR(".Update file is nonempty on Client!");


	return 0;
}
////////////////////////////////////////////////////////////////////////



//[3.5] PUSH//////////////////////////////////////////////////////////////
void* pushServer(  int sockfd, char* proj_name  ){


	return 0;
}
////////////////////////////////////////////////////////////////////////


//[3.6] CREATE//////////////////////////////////////////////////////////////
void* createServer(  int sockfd, char* proj_name ){
	printf("\n\tEntered command: create\n");
	/*error check*/
	if( typeOfFile(proj_name)==isDIR ){ sendErrorSocket(sockfd); pRETURN_ERROR("project already exists on server",NULL); }


	/*make directory*/
	if( mkdir( proj_name , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){ pRETURN_ERROR("mkdir()", NULL); }

	/*make .Manifest File*/
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	int manifest_fd = openFileW( manifest_path );
		if( manifest_fd < 0){ free(manifest_path); pRETURN_ERROR("open", NULL); }
	WRITE_AND_CHECKn(manifest_fd, "1\n", 2);

	/*make backup directory*/
	char* backup_proj_dir = concatString(proj_name, ".bak");
	if( mkdir( backup_proj_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ){ free(manifest_path); pRETURN_ERROR("mkdir()", NULL); }

	/*send Manifest file to client*/
	if( sendTarFile( sockfd, manifest_path, backup_proj_dir) == false){ free(backup_proj_dir); free(manifest_path); pRETURN_ERROR("sending .Manifest file to client", NULL); }

	//TODO: MAKE LINKED LIST

	//free and return
	printf("\n\tSuccessfully created project: %s on Server side!\n", proj_name);
	free(backup_proj_dir);
	free(manifest_path);
	close(manifest_fd);

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* destroyServer(  int curr_sockid, char* proj_name  ){

	//node to lock
	//ProjectNode* lock_Node = search(proj_name);
	//TODO lock repository, expire pending commits

	//Pointer for directory
	struct dirent *de;

	if( typeOfFile(proj_name)!=isDIR ){pRETURN_ERROR(("Project does not exist"), NULL); }

	//Opening the directory of path given
	DIR *dr = opendir(proj_name);
		if(!dr) pRETURN_ERROR("not a directory", false);

	while((de = readdir(dr)) !=NULL){
		if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0){ continue; }

		//Finding name of file and cending path back into method incase of being a directory
		char* new_path = combinedPath(proj_name, de->d_name);
		int np_type = typeOfFile(new_path);

		if( np_type  == isDIR ){
			//if file is directory, recurse to enter
			destroyServer(curr_sockid, new_path);
		}
		else{
			unlink(new_path);
		}

		//freeing
		free(new_path);
	}
	if ( sendFileSocket(curr_sockid, "Files and directories have been deleted") == false);

	//closing, and returning
	closedir(dr);
	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

void* addServer(  int sockfd, char* proj_name, char* file_name ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

void* removeServer( int sockfd, char* proj_name, char* file_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

void* currentversionServer(  int sockfd, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* historyServer( int sockfd, char* proj_name  ){
	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* rollbackServer(  int sockfd, char* proj_name, char* version_num){

	return 0;
}
////////////////////////////////////////////////////////////////////////



//CONNECT//////////////////////////////////////////////////////////////////////
//Handles accepting information sent in by the client
void* connect_client(void* curr_socket ){
	int sockfd = *(int*)curr_socket;
	printf("%d] Success on connection to client %d!\n", sockfd, num_clients);

	//Recieve number of bytes to read from client
		int num_bytes;
		READ_AND_CHECKn( sockfd , &num_bytes, 4);
			//if error
			if(num_bytes <= 0){ printf("\tError on Client Side recieving args\n"); return 0; }
		printf("\tRecieved from client - num_bytes_toread: %d\n", num_bytes );


	//Recieve info from client
		char* info_from_client = (char*)malloc(num_bytes + 1);
				if( info_from_client==NULL ) pEXIT_ERROR("malloc");
		READ_AND_CHECKn( sockfd , info_from_client , num_bytes);
		info_from_client[num_bytes] = '\0';

	//Parse through info and store in variables
		char delim[1]; delim[0] = (char)176;
		char* command = strtok(info_from_client, delim);
		char* proj_name = strtok(NULL, delim);
		char* s3 = strtok(NULL, delim); //file or version_num if applicable
		printf("\tRecieved from client - command:%s  proj_name:%s  extra_info?:%s\n", command, proj_name, s3);



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

	else if(strcmp(command,"add")==0)
		addServer(sockfd, proj_name, s3);

	else if(strcmp(command,"remove")==0)
		removeServer(sockfd, proj_name, s3);

	else if(strcmp(command,"currentversion")==0)
		currentversionServer(sockfd, proj_name);

	else if(strcmp(command,"history")==0)
		historyServer(sockfd, proj_name);

	else if(strcmp(command,"rollback")==0)
		rollbackServer(sockfd, proj_name, s3);

	else
		printf("\tError on client side\n");

	//TODO delete(use as reference for methods)
	//send to socket
	//send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 );

	//freeing and exiting
	/*
	shutdown(overall_socket,0);
	shutdown(overall_socket,1);
	shutdown(overall_socket,2);
	*/

	//close

	if(close(sockfd) < 0) pRETURN_ERROR("Error on Close", NULL);

	//pthread_exit(NULL);

	return 0;
}
////////////////////////////////////////////////////////////////////////



int main(int argc, char * argv[]){ //TODO: print out error message?
	//Check for arguments
		if(argc!=2) pRETURN_ERROR("Enter an argument containing the port number\n",-1);

	//getting port
		int port = (int)atol(argv[1]);

	//server address
		struct sockaddr_in address;

	//CREATING SOCKET
	overall_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(overall_socket == 0) pRETURN_ERROR("Error on socket creation",-1);
	//reuse socket
		if (setsockopt(overall_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)  pRETURN_ERROR("setsockopt(SO_REUSEADDR) failed",-1);

	//initialize overall_socket


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
	 //add client thread to global
	 	ClientThread curr_client = {-1, curr_socket};
		clients[num_clients] =  curr_client; //TODO: change -1 to pthread

		num_clients++; //TODO: add mutex
		//connect to client-
		connect_client( (void*)&curr_socket );
	}

	//if accept failed
	if(curr_socket<0){ pRETURN_ERROR("Connection to client failed",-1); }

/*TODO clients SHOULD BE INSIDE LOOP
		//create new thread
		pthread_t id;
		status = pthread_create(&id, NULL, connect_client, (void*) &tempSocket);
			if(status<0) pRETURN_ERROR("Thread not created",-1);
		//insert id into linked list
		insert(id);
	}
*/

/*
	//SHUT DOWN AND RETURN
	shutdown(overall_socket,0);
	shutdown(overall_socket,1);
	shutdown(overall_socket,2);
*/
	if(close(overall_socket) < 0) pRETURN_ERROR("Error on Close",-1);
	return 0;
}
