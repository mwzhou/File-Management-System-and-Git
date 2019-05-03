#ifndef STRUCT_H
#define STRUCT_H

#include "fileHelperMethods.h"
#define heightManifestTree(node) (((node)==NULL)? 0: (node)->height)
/*
Manifest ManifestTree Tree to order items in the manifest
*/
 typedef struct ManifestNode{
	 char* file_name;
	 int fver_num;
	 char* hash;
	 int mver_num;
	 bool compared;

	 int height;
	 struct ManifestNode* left;
	 struct ManifestNode* right;
 }ManifestNode;


 //Struct for linked list of ip adresses of pthreads being created
 typedef struct ProjectNode{
 	char* project_name;
 	pthread_mutex_t lock;
 	struct ProjectNode *next;
 }ProjectNode;


//METHOD SIGS
	ManifestNode* createManifestNode(char* file_name, int fver_num, char* hash, int mver_num);
	ManifestNode* insertManifestTree(ManifestNode* root, char* file_name, int fver_num, char* hash, int mver_num);
	ManifestNode* buildManifestTree(char* manifest_path);
	char* getFileManifestTree( ManifestNode* root, char* key);

	ManifestNode* BalanceManifestTree(ManifestNode* root, int balance_factor, char* key);
	void Case1Balance(ManifestNode** root_ptr, bool isLeft);
	void Case2Balance(ManifestNode** root_ptr, bool isLeftRight);
	int greaterHeight(ManifestNode* parent);
	int sizeOfManifestTreeTree(ManifestNode* root);
	void freeManifestTreeTree(ManifestNode* root);

	bool addProjectNodePN(ProjectNode* head, char* proj_name);
	bool delProjectNodePN(ProjectNode* head, char* proj_name);
	ProjectNode* searchProjectNodePN(ProjectNode* head, char* proj_name);

#endif
