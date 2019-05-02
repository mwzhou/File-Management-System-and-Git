#ifndef SERVER_H
#define SERVER_H

#include "fileHelperMethods.h"

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