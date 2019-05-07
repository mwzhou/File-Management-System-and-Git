#ifndef STRUCT_H
#define STRUCT_H

#include "fileHelperMethods.h"
#define heightManifestTree(node) (((node)==NULL)? 0: (node)->height)
/*
Manifest ManifestNode Linked List
*/
 typedef struct ManifestNode{
   int mver_num;
	 char* file_name;
	 int fver_num;
	 char* hash;

   struct ManifestNode* next;
 }ManifestNode;


 //Struct for linked list of ip adresses of pthreads being created
 typedef struct ProjectNode{
 	char* proj_name;
 	pthread_mutex_t lock;
 	struct ProjectNode *next;
 }ProjectNode;


 //STRUCTS
 typedef struct ClientThread{
 	pthread_t client;
 	int curr_socket;
  bool done;
 }ClientThread;


//METHOD SIGS
  ManifestNode* createManifestNode(	int mver_num, char* file_name, int fver_num, char* hash);
  ManifestNode* buildManifestLL(char* manifest_path);
  ManifestNode* searchManifestNode(ManifestNode* head, char* file_name);
  bool addManifestNode( ManifestNode** head_addr, ManifestNode* to_add );
  bool delManifestNode(ManifestNode** head_addr, char* file_name);
	bool delLLIfEmpty( ManifestNode** head_addr );

	bool addProjectNodePN(ProjectNode** head, char* proj_name);
	bool delProjectNodePN(ProjectNode** head, char* proj_name);
	ProjectNode* searchProjectNodePN(ProjectNode* head, char* proj_name);

  void printManifestNode( ManifestNode* head );
  void printProjectNode( ManifestNode* head );

#endif
