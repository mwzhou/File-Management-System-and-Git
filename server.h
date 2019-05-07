#ifndef SERVER_H
#define SERVER_H

#include "fileHelperMethods.h"

	//MACROS///////////////////////////////////////////////////////////////////////
	#define recieveStringSocket(sockfd) recieveStringSocketst( sockfd, "Client")
	#define sendStringSocket(sockfd, str) sendStringSocketst(sockfd, str, "Client")

	#define recieveFileSocket( sockfd, modify_fname ) recieveFileSocketst( sockfd, modify_fname, "Client" )
	#define sendFileSocket( sockfd, file_name )  sendFileSocketst(sockfd, file_name, "Client")

	#define sendTarFile( sockfd, file_path, dir_to_store)	sendTarFilest( sockfd, file_path, dir_to_store,  "Client" )
	#define recieveTarFile( sockfd, dir_to_store) recieveTarFilest( sockfd, dir_to_store , "Client")

	#define addProjectNode( proj_name ) addProjectNodePN(&head, proj_name)
	#define delProjectNode( proj_name ) delProjectNodePN(&head, proj_name)
	#define searchProjectNode( proj_name ) searchProjectNodePN(head, proj_name)
	/////////////////////////////////////////////////////////////////////////
	
	typedef struct manager_thread_args{
		sigset_t* set; //signal masks
		int sd; //listening socket 
	}manager_thread_args;

	pthread_mutex_t project_lock = PTHREAD_MUTEX_INITIALIZER;

	bool storeCurrentVersion(char* proj_name, char* backup_proj);
	bool updateServerOnPush( char* proj_name, char* dir_of_files, char* commitf_name );
	bool replaceManifestOnPush( char* proj_name, char* dir_of_files, char* commit_file );
	bool writeToHistory( char* proj_name , char* commit_client_path, FILE* history_fd);

#endif
