#ifndef SERVER_H
#define SERVER_H

#include "fileHelperMethods.h"

//STRUCTS
//Struct for linked list of ip adresses of pthreads being created
typedef struct ProjectNode{
	char* project_name;
	struct ProjectNode *next;
}ProjectNode;


typedef struct ClientThread{
	pthread_t client;
	int curr_socket;
}ClientThread;





#endif
