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
/////////////////////////////////////////////////////////////////////////


//STRUCTS
//Struct for linked list of ip adresses of pthreads being created
typedef struct ProjectNode{
	char* project_name;
	pthread_mutex_t lock;
	struct ProjectNode *next;
}ProjectNode;


typedef struct ClientThread{
	pthread_t client;
	int curr_socket;
}ClientThread;

//linked List Methods
bool addNode(char* proj_name);
bool delNode(char* proj_name);
ProjectNode* search(char* proj_name);


#endif
