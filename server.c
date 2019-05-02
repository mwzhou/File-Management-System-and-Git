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
int socket_c;

ClientThread clients[20];
int num_clients = 0;

ProjectNode *head = NULL;


//MACROS///////////////////////////////////////////////////////////////////////
#define recieveStringSocket(sockfd) recieveStringSocketst( sockfd, "Client")
#define sendStringSocket(sockfd, str) sendStringSocketst(sockfd, str, "Client")
#define recieveFileSocket( sockfd, modify_fname ) recieveFileSocketst( sockfd, modify_fname, "Client" )
#define sendFileSocket( sockfd, file_name )  sendFileSocketst(sockfd, file_name, "Client")
/////////////////////////////////////////////////////////////////////////

//////Linked List Methods////////////////////////////////////////////////////////////////


//initializes head and adds node to start of linked list
bool addNode(char* proj_name){
	ProjectNode *temp = malloc(sizeof(ProjectNode));
	temp->project_name = proj_name;
	//initialize mutex lock 
	int ret = pthread_mutex_init(&temp->lock, NULL);
	if(ret<0) {pRETURN_ERROR("Mutex Initialize", NULL); }

	if(head==NULL){
		head = temp;
		temp->next = NULL;
	}
	else{
		temp->next = head;
		head = temp;
	}
	return true;
}


//Deletes a node, returs true if found and deleted, returns false if not found
bool delNode(char* proj_name){
	ProjectNode* prev = NULL;
	ProjectNode *temp = head;
	while((temp!=NULL) && (strcmp(temp->project_name,proj_name)!=0)){
		prev = temp;
		temp = temp->next;
	}
	if(prev==NULL && (strcmp(temp->project_name,proj_name)==0)){
		head = temp->next;
		return true;
	}
	else if(temp->next==NULL && (strcmp(temp->project_name,proj_name)==0)){
		prev->next  = NULL;
		return true;
	}
	else if((strcmp(temp->project_name,proj_name)==0)){
		prev->next = temp->next;
		return true;
	}
	return false;
}


//Returns node of project when given project name to find
ProjectNode* search(char* proj_name){
	ProjectNode *temp = head;
	while(temp!=NULL){
		if(strcmp(temp->project_name,proj_name)==0)
			return temp;;
	}
	return NULL;
}


//[3.1] CHECKOUT////////////////////////////////////////////////////////////////////////

/*
Accepts project name as an incoming request from client and sends back current version of project
*/
void* checkoutServer( int curr_sockid, char* proj_name ){
	printf("%d] Entered command: checkout\n", curr_sockid);
	return 0;
}
////////////////////////////////////////////////////////////////////////


//[3.2] UPDATE//////////////////////////////////////////////////////////////////////

/*
update
*/
void* updateServer(  int curr_sockid, char* proj_name  ){
	printf("%d] Entered command: update\n", curr_sockid);

	//error check
		//check if project name exists
		if( typeOfFile(proj_name)!=isDIR ){ sendErrorSocket(curr_sockid); pRETURN_ERROR("project doesn't exist on server",NULL); }
		//check if manifest exists
		char* manifest_path = combinedPath( proj_name, ".Manifest"); //get path of manifest
		if( typeOfFile(manifest_path) != isREG ){  free(manifest_path); sendErrorSocket(curr_sockid); pRETURN_ERROR(".Manifest file doesn't exist in project on server",NULL); }

	//send manifest file to client
		if ( sendFileSocket(curr_sockid, manifest_path) == false){ pRETURN_ERROR("error sending .Manifest file", NULL); }


/*
		char* manifest_tar_path = makeTar(manifest_tar_path);
			if(manifest_tar == NULL){ sendErrorSocket(curr_sockid); return NULL; }
		char* manifest_str = readFile(manifest_tar_path);
		if( sendStringSocket(curr_sockid, manifest_tar) == false ){ pRETURN_ERROR("error sending",NULL); }

	free(manifest_path);
*/
	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* upgradeServer(  int curr_sockid, char* proj_name  ){
	//error check
		if( typeOfFile(proj_name)!=isDIR ){ sendErrorSocket(curr_sockid); pRETURN_ERROR("project doesn't exist on server",NULL); }

	//TODO:


	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* commitServer( int curr_sockid, char* proj_name ){


	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* pushServer(  int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* createServer(  int curr_sockid, char* proj_name ){

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
void* addServer(  int curr_sockid, char* proj_name, char* file_name ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* removeServer( int curr_sockid, char* proj_name, char* file_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* currentversionServer(  int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* historyServer( int curr_sockid, char* proj_name  ){

	return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void* rollbackServer(  int curr_sockid, char* proj_name, char* version_num){

	return 0;
}
////////////////////////////////////////////////////////////////////////



//CONNECT//////////////////////////////////////////////////////////////////////
//Handles accepting information sent in by the client
void* connect_client(void* curr_socket ){
	int curr_sockid = *(int*)curr_socket;
	printf("%d] Success on connection to client %d!\n", curr_sockid, num_clients);

	//Recieve number of bytes to read from client
		int num_bytes;
		READ_AND_CHECKn( curr_sockid , &num_bytes, 4);
			//if error
			if(num_bytes <= 0){ printf("\tError on client side\n"); return 0; }
		printf("\tRecieved from client - num_bytes_toread: %d\n", num_bytes );


	//Recieve info from client
		char* info_from_client = (char*)malloc(num_bytes + 1);
				if( info_from_client==NULL ) pEXIT_ERROR("malloc");
		READ_AND_CHECKn( curr_sockid , info_from_client , num_bytes);
		info_from_client[num_bytes] = '\0';

	//Parse through info and store in variables
		char delim[1]; delim[0] = (char)176;
		char* command = strtok(info_from_client, delim);
		char* proj_name = strtok(NULL, delim);
		char* s3 = strtok(NULL, delim); //file or version_num if applicable
		printf("\tRecieved from client - command:%s  proj_name:%s  extra_info?:%s\n", command, proj_name, s3);




	//The following if statements call methods based on the request sent from the client
	if(strcmp(command,"checkout")==0)
		checkoutServer(curr_sockid, proj_name);

	else if(strcmp(command,"update")==0)
		updateServer(curr_sockid, proj_name);

	else if(strcmp(command,"upgrade")==0)
		upgradeServer(curr_sockid, proj_name);

	else if(strcmp(command,"commit")==0)
		commitServer(curr_sockid, proj_name);

	else if(strcmp(command,"push")==0)
		pushServer(curr_sockid, proj_name);

	else if(strcmp(command,"create")==0)
		createServer(curr_sockid, proj_name);

	else if(strcmp(command,"destroy")==0)
		destroyServer(curr_sockid, proj_name);

	else if(strcmp(command,"add")==0)
		addServer(curr_sockid, proj_name, s3);

	else if(strcmp(command,"remove")==0)
		removeServer(curr_sockid, proj_name, s3);

	else if(strcmp(command,"currentversion")==0)
		currentversionServer(curr_sockid, proj_name);

	else if(strcmp(command,"history")==0)
		historyServer(curr_sockid, proj_name);

	else if(strcmp(command,"rollback")==0)
		rollbackServer(curr_sockid, proj_name, s3);

	else
		printf("\tError on client side\n");

	//TODO delete(use as reference for methods)
	//send to socket
	//send(socket , "Message recieved from server!" , strlen("Message recieved from server!") , 0 );

	//freeing and exiting
	/*
	shutdown(socket_c,0);
	shutdown(socket_c,1);
	shutdown(socket_c,2);
	*/

	//close
	if(close(curr_sockid) < 0) pRETURN_ERROR("Error on Close", NULL);
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
	socket_c = socket(AF_INET, SOCK_STREAM, 0);
		if(socket_c == 0) pRETURN_ERROR("Error on socket creation",-1);
	//reuse socket
		if (setsockopt(socket_c, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)  pRETURN_ERROR("setsockopt(SO_REUSEADDR) failed",-1);

	//initialize socket_c


	//intitializes sizeof(address) zero-value bytes of address
	bzero(&address,sizeof(address));

	//initializes sockaddr_in functions for address
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	//BINDING to client
	int addrlen = sizeof(address);
	int status = bind(socket_c, (struct sockaddr*) &address, addrlen);
		if(status < 0) pRETURN_ERROR("Error on Bind",-1);

	//LISTENto client
	status = listen(socket_c, 20);
		if(status < 0) pRETURN_ERROR("Error on Listen",-1);


	//ACCEPT connecting and accepting message for client
	int curr_socket;
	while( (curr_socket= accept(socket_c, (struct sockaddr*) &address, (socklen_t*)&addrlen)) > 0 ){
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
	shutdown(socket_c,0);
	shutdown(socket_c,1);
	shutdown(socket_c,2);
*/
	if(close(socket_c) < 0) pRETURN_ERROR("Error on Close",-1);
	return 0;
}
