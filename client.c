#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>
#include<errno.h>
#include <math.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <openssl/sha.h>
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

	//WRITE IP and Port info to file#include <openssl/sha.h>
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
			printf("\tRecieving Manifest file from Server\n");
			char* store_manifest_serv_dir = concatString(proj_name, ".bak");  //make path to store manifest file recieved from Server
			char* manifest_serv_path = recieveTarFile(sockfd, store_manifest_serv_dir);
				free(store_manifest_serv_dir);
				if( manifest_serv_path == NULL ) pEXIT_ERROR("recieving Manifest File");

		//get manifest file for client
			char* manifest_client_path = combinedPath( proj_name, ".Manifest" );

		/*Get items!*/
				//get path for .Update file
				char* update_path = combinedPath( proj_name, ".Update" );
				//create file
				int update_fd = openFileW( update_path );
					if( update_fd < 0 ){ 	free(manifest_client_path); free(manifest_serv_path); free(update_path ); pEXIT_ERROR("update"); }
  
		/* Comparison Linked Lists should be free after write*/
			ManifestNode* clientLL = buildManifestLL( manifest_client_path );
				free(manifest_client_path);
				if( clientLL == NULL ){ pEXIT_ERROR("error creating Manifest Linked List (Client)"); }
			ManifestNode* serverLL = buildManifestLL( manifest_serv_path );
				free(manifest_serv_path);
				if( serverLL == NULL ){ pEXIT_ERROR("error creating Manifest Linked List (Server)"); }


	/*WRITE UPDATE FILE AND DO COMPARISONS*/
	/*Comparisons - writes update file and makes comparisons with two manifest trees*/
		printf("\n\tWill start writing .Update file: listing all updates...\n");

	//write update file
		bool write_success = writeUpdateFile( clientLL , serverLL, update_fd );
		if( write_success == false ){
			REMOVE_AND_CHECK(update_path);
			printf("\tUpdate conflict!\n");
		}else{
			if( sizeOfFile(update_path) == 0 ){
				printf("\t\tCompletely up to date with Server. No updates neccessary\n");
			}
		}

	//free and return
		close( update_fd );
		free(update_path);
		return;
}


/**
FREES LINKED LISTS THROUGH DELETES
goes through each manifest - compares files and deletes if not found
**/
bool writeUpdateFile( ManifestNode* clientLL_head , ManifestNode* serverLL_head , int update_fd ){
	int clienth_mver_num = clientLL_head->mver_num; //manifest version client
	int serverh_mver_num = serverLL_head->mver_num; //manifest version server

	/*Comparison Client to Server*/
	ManifestNode* client_ptr = clientLL_head;

	while( client_ptr != NULL){
		char* up_cmd = NULL;
		char* cptr_file = client_ptr->file_name;
		ManifestNode* serv_cmpnode = searchManifestNode( serverLL_head, cptr_file );

		/**COMPARISONS*/
		//both LLs have a file entry
		if(serv_cmpnode != NULL){
			char* cptr_livehash = generateHash( cptr_file );
				if( cptr_livehash == NULL ){ pRETURN_ERROR("generate live hash", false); }

			//COMPARISON
			//same manifest version number, but different hashes from server and live
				if( (serv_cmpnode->mver_num == client_ptr->mver_num) &&  (strcmp( cptr_livehash , serv_cmpnode->hash) != 0) ){
					up_cmd = "U";

			//diff manifest version number, diff file version number, same hash live hash and client
				}else if ( (serv_cmpnode->mver_num != client_ptr->mver_num) &&  (strcmp( cptr_livehash , client_ptr->hash) == 0)  && (serv_cmpnode->fver_num != client_ptr->fver_num) ){
				   	up_cmd = "M";

				//if exactly the same
				}else if( (serv_cmpnode->mver_num == client_ptr->mver_num) &&  (strcmp( cptr_livehash , serv_cmpnode->hash) == 0)  && (serv_cmpnode->fver_num == client_ptr->fver_num)  ){
						up_cmd = "N";
				}
			//free
			free(cptr_livehash);

		 //Only Client has this file
		}else{
				 //if manifest version number is same
				 if( (serverh_mver_num == client_ptr->mver_num) ){
					 up_cmd = "U";
				 //if manifest version number is DIFFERENT
			 	}else{
				 	 up_cmd = "D";
				}
		}


		/*WRITING TO FILE*/
		if(up_cmd == NULL){
				printf("\t\tCONFLICT ERROR:\t%5s\n", cptr_file);
				return false;
		}else if( up_cmd[0]!= 'N'){
			//write to file
			WRITE_AND_CHECKb( update_fd, cptr_file , strlen(cptr_file) );
			WRITE_AND_CHECKb( update_fd, "\t" , 1 );
			WRITE_AND_CHECKb( update_fd, up_cmd , 1 );
			WRITE_AND_CHECKb( update_fd, "\n" , 1 );

			printf("\t\tUpdate Command:%-5s\tFile:%-5s\n", up_cmd, cptr_file);
		}

		//UPDATE
		client_ptr = client_ptr->next;

		/*DELETE NODES*/
		if(serv_cmpnode != NULL) delManifestNode( &serverLL_head, cptr_file);
		delManifestNode( &clientLL_head, cptr_file);
	}
  
  
	/*Comparison rest of Server files*/
	ManifestNode* server_ptr = serverLL_head;
		if( server_ptr == NULL ) return true;


	while( server_ptr!= NULL ){
		char* sptr_file = server_ptr->file_name;

		if( (server_ptr->mver_num != clienth_mver_num) ){
			//write to file
			WRITE_AND_CHECKb( update_fd, sptr_file , strlen(sptr_file) );
			WRITE_AND_CHECKb( update_fd, "\tA\n" , 3 );

			printf("\t\tUpdate Command:%-5s\tFile:%-5s\n", "A", sptr_file);

		}else{
			printf("\t\tCONFLICT ERROR:\t%-5s\n", sptr_file);
			return false;
		}

		//UPDATE
		server_ptr = server_ptr->next;

		/*DELETE NODES*/
		delManifestNode( &serverLL_head , sptr_file);
	}
	return true;


}
////////////////////////////////////////////////////////////////////////////


//[3.3] UPGRADE//////////////////////////////////////////////////////////////
void upgradeClient(char* proj_name){
	printf("%d] Entered command: upgrade\n", sockfd);

	sendArgsToServer("upgrade", proj_name, NULL);

	/*ERROR CHECK*/
		//waiting for signal if valid project on server
			if( receiveSig(sockfd) == false) pEXIT_ERROR("project doesn't exist on server");
		//check if update file exists on Client
			char* update_path = combinedPath(proj_name, ".Update");
			if( sendSig( sockfd, (typeOfFile(update_path)!=isREG )) == false ){ free(update_path); pEXIT_ERROR(".Update file doesn't exist on Client"); }
		//If .Update file is empty
			if( sendSig(sockfd, (sizeOfFile(update_path) <= 0)) == false ){ remove(update_path); free(update_path); printf("\t.Update is empty, project is up to date\n"); return; }


	/*OPERATIONS*/
	//opening update_file
	char* update_file = readFile(update_path);
		if( update_file == NULL ) pEXIT_ERROR("reading update");

	/*recieving files from Server through backup*/
		char* backup_proj_path = concatString( proj_name, ".bak" ); //get backup folder_dir
		char* serv_proj_path = recieveTarFile( sockfd, backup_proj_path);
			if(serv_proj_path == NULL) pEXIT_ERROR("error recieving directory");

	/*PARSE THROUGH FILE*/
		char* tok = strtok( update_file, "\n\t");
		do{
		//get file
			char* up_file = tok;

		//get update command
			tok = strtok(NULL, "\n\t");
			char* up_cmd = tok;
				if( strlen(up_cmd)!= 1){ removeDir( serv_proj_path ); pEXIT_ERROR("Invalid Update up_file"); }


		//perform operations based on command
			switch( up_cmd[0] ){
				case 'U':
					break;

				case 'M':
				case 'A':{ //replace
					//get server version
					char* fserv_version = combinedPath(backup_proj_path, up_file);
					//get directory to store
					int ind_slash = lengthBeforeLastOccChar( up_file, '/');
					char* dir_to_store = substr(up_file, 0, ind_slash+1);

					//REPLACE FILE
					if( moveFile( fserv_version , dir_to_store) == false){
						free(fserv_version); free(dir_to_store); removeDir( serv_proj_path );
						free(update_path); free(serv_proj_path); free(backup_proj_path); free(update_file);
						printf("\n\tFILE: %s CMD: %s\n", up_file, up_cmd);
						pEXIT_ERROR("move or add");
					}

					free(fserv_version);
					free(dir_to_store);
					break;
          
				}case 'D':
					//REMOVE FILE
					remove(up_file);
					break;

				default:
					removeDir( serv_proj_path );
					pEXIT_ERROR("Invalid Update file");
			}

		//printf("FILE %s CMD %s\n\n", up_file, up_cmd);
		}while( (tok = strtok(NULL, "\n\t")) != NULL );

	//replace Manifest w/ Server's manifest
		//get server version
		char* fserv_manifest = combinedPath(backup_proj_path, ".Manifest");
		if( moveFile( fserv_manifest , proj_name ) == false){
				PRINT_ERROR("move");
		}

	//remove and free
	removeDir( serv_proj_path );
	remove( update_path );
	free( fserv_manifest );
	free(update_path);
	free(serv_proj_path);
	free(backup_proj_path);
	free(update_file);
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.4] COMMIT//////////////////////////////////////////////////////////////
void commitClient(char* proj_name){
	printf("%d] Entered command: commit\n", sockfd);
	sendArgsToServer("commit", proj_name, NULL);

	/**ERROR CHECK**/
		//check if project name doesn't exist on Server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project does not exist on Server");
		//check if manifest doesn't exist on Server
		if( receiveSig(sockfd) == false ) pEXIT_ERROR(".Manifest file does not exist on Server");
		//check if manifest doesn't exist on client
		char* client_manifest = combinedPath( proj_name, ".Manifest" );
		if( sendSig(sockfd,  (typeOfFile(client_manifest)!= isREG ) ) == false ){ pEXIT_ERROR(".Manifest does not exist on Client"); }

	//retrieve manifest from server
		char* backup_proj = concatString( proj_name, ".bak" );
		printf("\n\tRecieving Manifest file from Server...\n");
		char* server_manifest = recieveTarFile( sockfd, backup_proj );
			if( server_manifest == NULL ) pEXIT_ERROR("retireving server's Manifest");

	/*Create Linked Lists for both Manifests*/
		ManifestNode* clientLL = buildManifestLL( client_manifest );
		ManifestNode* serverLL = buildManifestLL( server_manifest );

		//check if manifest versions are the same
		if( sendSig(sockfd, (clientLL->mver_num != serverLL->mver_num ) ) == false){
			printf("\tManifest Versions are different, waiting for user to update Manifest Version\n");
			return;
		}

		/*CREATING Commit file*/
		char* commit_path = combinedPath(proj_name,".Commit");
		FILE* commit_fd = fopen( commit_path , "w" );
			if( commit_fd == NULL ) pEXIT_ERROR("open .Commit File");

		/*WRITE COMMIT FILE*/
		printf("\n\tWill start writing .Commit file: listing all commits...\n");

		//if failed to write commit file
		if( sendSig( sockfd, (!writeCommitFile( clientLL, serverLL, commit_fd))) == false ){
			REMOVE_AND_CHECK( commit_path );
			printf("\tCOMMIT FAIL please resynch repository\n");
		//success
		}else{
			if( sizeOfFile(commit_path) == 0){
				printf("\t\tNo commits to do, completely up to date with server!\n\n");
			}
			//send tar file to server
			printf("\tSending Commit file to Server...\n");
			if( sendTarFile( sockfd, commit_path, backup_proj) == false )
				printf("\terror sending commit file\n");
		}

		//FREE
		fclose(commit_fd);
		free(commit_path);
		free(client_manifest);
		free(backup_proj);
		free(server_manifest);
}


/**
Check Commit Validity
**/
bool writeCommitFile( ManifestNode* clientLL_head, ManifestNode* serverLL_head, FILE* commit_fd ){
	char* commit_cmd = NULL;

	/*Compare Client to Server*/
	ManifestNode* cptr = clientLL_head;
	while( cptr!=NULL ){
		char* cptr_fname = cptr->file_name;

		//Compare hashes to live hash and get appropriate version number
		char* cptr_livehash = generateHash(cptr_fname);
			if( cptr_livehash == NULL ) return false;
		//if hashes are different, increment the file version number
		int cptr_newv_num = (strcmp( cptr_livehash, cptr->hash ) != 0) ? cptr->fver_num+1 : cptr->fver_num;

		ManifestNode* found_servnode = searchManifestNode( serverLL_head, cptr_fname );
		//if new client is greater
		if( found_servnode != NULL && (cptr_newv_num > found_servnode->fver_num) ){
			commit_cmd = "U";
		//in client's manifest but not in servers
		}else if( found_servnode == NULL ){
			commit_cmd = "A";
		//in both .Manifests and exactly the same
	}else if( found_servnode != NULL && (cptr_newv_num == found_servnode->fver_num) && (strcmp(cptr_livehash, found_servnode->hash)==0)){
			commit_cmd = "N"; //ignore
		}


		/*OUTPUT*/
		if( commit_cmd == NULL){
			printf("\t\tCONFLICT ERROR:\t%5s\n", cptr_fname);
			return false;
		}else if( commit_cmd[0] != 'N'){
			fprintf( commit_fd , "%s\t%s\t%d\t%s\n" , cptr_fname, commit_cmd, cptr_newv_num, cptr_livehash);
			printf("\t\tCommit Command:%-5s\tFile:%-5s\n", commit_cmd, cptr_fname);
		}

		//UPDATE client ptr
		cptr = cptr-> next;
		free(cptr_livehash);

		//DELETE entry from both lists
		if(found_servnode != NULL) delManifestNode( &serverLL_head, cptr_fname);
		delManifestNode( &clientLL_head, cptr_fname);
	}


	/*Compare Server to rest of Client*/
	if( serverLL_head == NULL ) return true; //if no files solely on server
	ManifestNode* sptr = clientLL_head;
	while( sptr != NULL ){
		char* sptr_fname = sptr->file_name;
		//write
		fprintf( commit_fd , "%s\tR\t%d\n" , sptr_fname, sptr->fver_num );
		printf("\t\tCommit Command:%-5s\tFile:%-5s\n", "R", sptr_fname);
		//update
		sptr = sptr->next;

		//delete
		delManifestNode( &serverLL_head, sptr_fname);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////


//[3.5] PUSH//////////////////////////////////////////////////////////////
void pushClient(char* proj_name){
	printf("%d] Entered command: push\n", sockfd);
	sendArgsToServer("push", proj_name, NULL);

	/**ERROR CHECK**/
		//check if project name doesn't exist on Server
		if( receiveSig(sockfd) == false ) pEXIT_ERROR("project does not exist on Server");
		//check if manifest doesn't exist on Server
		if( receiveSig(sockfd) == false ) pEXIT_ERROR(".Commit file does not exist on Server");

	//check if clients .Upgrade file contains any Ms
	char* update_file = combinedPath(proj_name, ".Update");
	FILE* uF = fopen(update_file,"r");
	bool hasM = false;
	if(uF){
		int lineSize = 1024;
		char buffer[lineSize];
		//going update file and searching for M codes
		while((fgets(buffer, lineSize, uF) )!=NULL){
			char* start = strstr(buffer,"\t");
			int index = start-buffer;
			char* code = substr(buffer, index+1, 2);
			if(strcmp(code,"M")==0){
				hasM = true;
				free(code);
				break;
			}
			free(code);
		}
		fclose(uF);
	}
	if( sendSig(sockfd, ( hasM==true ) ) == false )pEXIT_ERROR("A file was modified since the last upgrade");


	//Create a directory within the project which will be used to send files over	
	char* dir_to_send = concatString(proj_name,".to_send");
	struct stat st = {0};
	//check if directory exists, otherwise create it
	if(stat(dir_to_send, &st) == -1){
		mkdir(dir_to_send, 0700);
	}

	//getting commit file	
	char* commit_file = combinedPath(proj_name, ".Commit");
	FILE* cF = fopen(commit_file,"r");

	//setting initial variables	
	int lineSize = 1024;
	char buffer[lineSize];

	//going commit file and storing them under directory to be sent to server
	while((fgets(buffer, lineSize, cF) )!=NULL){
	
		//get complete file name
		char* end = strstr(buffer,"\t");
		int index_end = end-buffer;
		char* file_path = substr(buffer, 0, index_end+1);

		//get single file name
		int index_start = lengthBeforeLastOccChar( buffer , '/');
		char* file_name = substr(buffer, index_start+1, (index_end-index_start));
		char* copy_path = combinedPath(dir_to_send,file_name);

		copyDir(file_path , copy_path);

		free(file_path);
		free(file_name);
	}

	//close and add .Commit and .Manifest in file to server	
	fclose(cF);
	char* copy_path = combinedPath(dir_to_send,".Commit");
	copyDir( commit_file , copy_path);
	char* copy_path2 = combinedPath(dir_to_send, ".Manifest");
	char* clientManifest = combinedPath(proj_name, ".Manifest");
	copyDir(clientManifest, copy_path2);

	//sending file
	char* bakup_proj = concatString(proj_name, ".bak");
	if (sendTarFile( sockfd, dir_to_send, bakup_proj) == false) {pRETURN_ERRORvoid("sendTarFile failed");}

	//recieving error if .Commits were not the same and deleting .Commit file and exiting
	if( receiveSig(sockfd) == false ) {
		bool delete_file_dir = removeDir(dir_to_send);
		if(delete_file_dir==false){pRETURN_ERRORvoid("Removing directory failed");}
		//bool delete_commit = removeDir(commit_file);
		//if(delete_commit==false){pRETURN_ERRORvoid("Removing directory failed");}
		free(dir_to_send);
		free(clientManifest);
		free(copy_path);
		free(commit_file);
		free(bakup_proj);
		pEXIT_ERROR(".Commits were not the same");
	}

	//freeing and deleting directory
	bool delete_file_dir = removeDir(dir_to_send);
	if(delete_file_dir==false){pRETURN_ERRORvoid("Removing directory failed");}
	//bool delete_commit = removeDir(commit_file); //TODO uncomment
	//if(delete_commit==false){pRETURN_ERRORvoid("Removing directory failed");}
	free(dir_to_send);
	free(update_file);
	free(clientManifest);
	free(copy_path);
	free(copy_path2);
	free(commit_file);
	free(bakup_proj);

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

	//free and return
	printf("\n\tSuccessfully created project: %s on Client side!\n", proj_name);
	free(manifest_path);
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.7] DESTROY//////////////////////////////////////////////////////////////
void destroyClient(char* proj_name){
	/*ERROR CHECK*/
		//waiting for signal if valid project on server
		if( receiveSig(sockfd) == false ) pEXIT_ERROR("project doesn't exist on server");

	/*Wait for results of remove*/
	if( receiveSig(sockfd) == false ){
		pEXIT_ERROR("failed to remove on server");
	}else{
		printf("Successfully deleted project on server!");
	}

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

	/*ERROR CHECK*/
	//check valid arguments
	if ( typeOfFile(proj_name)!= isDIR ) pEXIT_ERROR("project name does not exist on client side");


	char* combined_path = combinedPath( proj_name, file_name );
	if(removeFromManifest(combined_path , proj_name)==false) pEXIT_ERROR("remove");

	free( combined_path );
	return;
}



bool removeFromManifest(char* file_path, char* proj_name){
	//Finding path of manifest file
		char* manifest_path = combinedPath(proj_name, ".Manifest");

	/*DELETING FROM FILE AND REWRITING*/
		//Files to read and rewrite in
			FILE* srcFile = fopen(manifest_path, "r");
				if( srcFile < 0 ) pRETURN_ERROR("open", false);

			char* temp_path = combinedPath( proj_name, "deleteFile.tmp" );
			FILE* tempFile = fopen( temp_path ,"w");
				if( tempFile < 0 ) pRETURN_ERROR("open", false);

		//finding line to delete
			int line = extractLine(manifest_path, file_path);

		//rewriting tempfile without line
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
		rename(temp_path,manifest_path);

	//free and close
	free(manifest_path);
	free(temp_path);
	fclose(srcFile);
	fclose(tempFile);

	return true;
}


////////////////////////////////////////////////////////////////////////////



//[3.10] CURRENT VERSION/////////////////////////////////////////////////////
void currentVersionClient(char* proj_name){
	printf("%d] Entered command: currentversion\n", sockfd);
	sendArgsToServer("currentversion", proj_name, NULL);

	/*ERROR CHECK*/
		//waiting for signal if valid project on server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project doesn't exist on server");
		if( receiveSig(sockfd) == false) pEXIT_ERROR(".Manifest doesn't exist on server");

	/*Recieve manifest str*/
			char* manifest_str = recieveStringSocket( sockfd );
				if( manifest_str == NULL ) pEXIT_ERROR("Recieving manifest string");

			printf("\n\tStarting to print out version numbers and file names:\n");

			/*parse through manifest file*/
			char* curr_token = strtok( manifest_str , "\n\t"); //get rid of new line

			char* file_name;
			char* fver_num;

			while( curr_token != NULL){
					//Get File_name
					file_name = strtok( NULL, "\n\t");
						if( file_name == NULL ) break;

					//get v_num
					fver_num =  strtok( NULL , "\n\t");
						if( fver_num == NULL ) pEXIT_ERROR("not correct .Manifest format");

					//output
					printf("\tversion_number: %-5s\tfile: %-5s\n", fver_num, file_name);

					curr_token = strtok( NULL , "\n\t");
			}

	free(manifest_str);
	return;
}
////////////////////////////////////////////////////////////////////////////



//[3.11] HISTORY//////////////////////////////////////////////////////////////
void historyClient(char* proj_name){
	printf("%d] Entered command: history\n", sockfd);
	sendArgsToServer("history", proj_name, NULL);

	/*ERROR CHECK*/
		//waiting for signal if valid project on server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project doesn't exist on server");
		if( receiveSig(sockfd) == false) pEXIT_ERROR(".History file doesn't exist on server");

	/*Recieving History file from Server*/
		printf("\tRecieving History file from Client...\n");
		char* backup_proj = concatString( proj_name, ".bak" );
		char* history_path = recieveTarFile( sockfd, backup_proj );
			if( history_path == NULL ){ free(backup_proj); free(history_path); pEXIT_ERROR("recieving History Tar file"); }
		printf("\tSuccessfully recieved file from Client...\n");

		char* hist_str = readFile( history_path );
			if( hist_str == NULL ){ pEXIT_ERROR("reading History file"); }

		//output history file
		printf("\n%s\n", hist_str );

	free(hist_str);
	free(backup_proj);
	free(history_path);
	return;
}
//////////////////////////////////////////////////////////////////////////////



//[3.12] ROLLBACK//////////////////////////////////////////////////////////////
void rollbackClient(char* proj_name, char* version){
	printf("%d] Entered command: rollback\n", sockfd);
	sendArgsToServer("rollback", proj_name, version);

	/*ERROR check*/
		//check if project exists on server
		if( receiveSig(sockfd) == false) pEXIT_ERROR("project doesn't exist on Server");
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

	//number of bytes
	int num_bytes = (strlen(s1) + 1 + strlen(s2) + 1);
		if(s3 != NULL) num_bytes += (strlen(s3) + 1);

	char delim[2]; delim[0] = (char)176; delim[1] = '\0';

	//get arguments
	char* arguments = (char*)malloc(num_bytes);
		if( arguments == NULL){ pRETURN_ERRORvoid("malloc"); }

		strcpy( arguments, s1 );
		strcat( arguments, delim );
		strcat( arguments, s2 );
		if( s3 != NULL ){
			strcat( arguments, delim );
			strcat( arguments, s3 );
		}

		sendStringSocket( sockfd, arguments );
		free(arguments);
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



