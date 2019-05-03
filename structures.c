
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include<errno.h>
#include <pthread.h>

#include "structures.h"

//ManifestNode methods (note this is the same type as TreeNode by typedef)////////////////////////////////////////////////
//(description of struct in structures.h)

/**
Initializes ManifestNode
**/
ManifestNode* createManifestNode(char* file_name, int fver_num, char* hash, int mver_num){
	ManifestNode* ret = (ManifestNode*)malloc(sizeof(ManifestNode));
	if( ret== NULL){ pEXIT_ERROR("malloc"); }

	ret->file_name = file_name;
	ret->fver_num = fver_num;
	ret->hash = hash;
	ret->mver_num = mver_num;
	ret->compared = false;

	ret->height = 1;
	ret->left = NULL;
	ret->right = NULL;

	return ret;
}



/** [private method]
Searches through ManifestTree tree recursively O(logn)
If found: Updates frequency and changes *op_ptr to UPDATED
If not found: Creates a Token element and inserts it into the tree.
MAINTAINS ManifestTree properties of the tree and balances if necessary
@params: ManifestNode* root - root of ManifestTree Tree,
		 char* file_name - file_name to insert/update,
		 int fver_num
		 char* hash
@returns: updated root after one insert/update
**/
 ManifestNode* insertManifestTree(ManifestNode* root, char* file_name, int fver_num, char* hash, int mver_num){
	if(root ==NULL)//no elements in the ManifestTreeTree yet
		return createManifestNode(file_name, fver_num, hash, mver_num);

	//Comparisons and Insert
	int strcmp_tok = strcmp(file_name, root->file_name);
	if(strcmp_tok<0){ //file_name passed in is lessthan root's file_name
		root->left = insertManifestTree((root->left) , file_name, fver_num, hash, mver_num);
	}else if(strcmp_tok>0){ //file_name passed in is less than root's file_name
		root->right = insertManifestTree((root->right) , file_name, fver_num, hash, mver_num);
	}else{
		pRETURN_ERROR("duplicate file", NULL);
	}

	//update height
	root->height = greaterHeight(root)+1;

	//Balancing the tree
	int balance_factor = (root==NULL)? 0 : (heightManifestTree(root->left)) - (heightManifestTree(root->right));
	root = BalanceManifestTree(root , balance_factor, file_name);
	return root;
}


/** [private method]
Balances ManifestTree Tree given a root after one insert (Constant time)
@params: ManifestNode* root - root of ManifestTreeTree;
		 int balance_factor - how imbalanced the tree is;
		 char* key  - token/encoding string to compare root->left and root->right to, to figure out what type of imbalance
@returns: updated pointer to balanced root.
**/
 ManifestNode* BalanceManifestTree(ManifestNode* root, int balance_factor, char* key){
	if(root==NULL||balance_factor==1|| balance_factor==0 ||key ==NULL) //can't compare
		return root;

	//initializes values of strcmp_left and strcmp_right based on comparison mode
	int strcmp_left = (root->left==NULL)? 0 : strcmp( key, (root->left)->file_name);
	int strcmp_right = (root->right==NULL)? 0 : strcmp( key, (root->right)->file_name);

	//Balancing - finds what type of imbalance root is
	if(balance_factor>1 && strcmp_left<0 ){ //Case: left-left
		Case1Balance(&root,true);
	}else if(balance_factor<-1 && strcmp_right>0 ){ //Case: right-right
		Case1Balance(&root,false);
	}else if(balance_factor>1 && strcmp_left>0 ){ //Case: left-right
		Case2Balance(&root,true);
	}else if(balance_factor<-1 && strcmp_right<0 ){ //Case: right-left
		Case2Balance(&root,false);
	}

	return root;
}


/**[private method]
Balances Tree if Case1: i.e. left-left or left-right
@params: isLeft - meaning is Case Left-Left
**/
 void Case1Balance(ManifestNode** root_ptr, bool isLeft){
	if(root_ptr==NULL||*root_ptr==NULL) return;

	//Case with Left-Left
	if (isLeft ){
		ManifestNode* x = (*root_ptr)->left;
		ManifestNode* T2 = x->right; //x's right subtree
		x->right = (*root_ptr);
		(*root_ptr)->left = T2;

		//update heights
		(*root_ptr)->height = greaterHeight((*root_ptr))+1;
		x->height = greaterHeight(x)+1;

		(*root_ptr) = x; //update root

	//Case with Right-Right
	}else{
		ManifestNode* x = (*root_ptr)->right;
		ManifestNode* T2 = x->left; //x's left-subtree
		x->left = (*root_ptr);
		(*root_ptr)->right = T2;

		//update heights
		(*root_ptr)->height = greaterHeight((*root_ptr))+1;
		x->height = greaterHeight(x)+1;

		(*root_ptr) = x; //update root
	}
}


/**[private method]
Balances Tree if Case2: i.e. left-right or right-left
@params: isLeftRight - meaning is Case Left-Right
**/
 void Case2Balance(ManifestNode** root_ptr, bool isLeftRight){
	if(root_ptr==NULL||*root_ptr==NULL) return;

	//Case with Left Right
	if(isLeftRight){
		//turn into Left-Left Case1
			ManifestNode* x = ((*root_ptr)->left)->right;
			ManifestNode* T2 = x->left; //x's left subtree
			x->left = (*root_ptr)->left;
			(*root_ptr)->left->right = T2;

			//update height
			(*root_ptr)->left->height = greaterHeight((*root_ptr)->left)+1;
			x->height = greaterHeight(x)+1;

			(*root_ptr)->left = x;

		Case1Balance(root_ptr, true);

	//Case with Right Left
	}else{
		//turn into Right-Right Case1
			ManifestNode* x = (*root_ptr)->right->left;
			ManifestNode* T2 = x->right; //x's right subtree
			(*root_ptr)->right->left = T2;
			x->right = (*root_ptr)->right;

			//update heights
			(*root_ptr)->right->height = greaterHeight((*root_ptr)->right)+1;
			x->height = greaterHeight(x)+1;

			(*root_ptr)->right = x;

		Case1Balance(root_ptr, false);
	}
}


/**[private method]
returns height that is greater between the children Nodes of the node passed in, accounts for if node passed in is NULL*/
 int greaterHeight(ManifestNode* parent){
	int left_h = heightManifestTree(parent->left);
	int right_h = heightManifestTree(parent->right);
	return (left_h > right_h)? left_h : right_h;
}


/**
gets number of nodes in an ManifestTree Frequency Tree
**/
int sizeOfManifestTree(ManifestNode* root){
	if(root==NULL)
		return 0;

	return 1+sizeOfManifestTree(root->left)+sizeOfManifestTree(root->right);
}


/**
frees all nodes in a ManifestTreeTree. PostOrder Traversal.
Note: DOES NOT free the Token element
**/
void freeManifestTree(ManifestNode* root){
	if(root==NULL) return;

	freeManifestTree(root->left);
	freeManifestTree(root->right);

	free(root->file_name);
	free(root->hash);
	free(root);
}


//Public ManifestTree methods/////////////////////

/**
builds ManifestTree based on the codebodetree = insertCodeTreeReook given. Note: assuming manifest_path is valid
if:
@returns MANTREE if successful
 returns NULL if error
 returns EMPTY NODE if manifest has no entries
**/
ManifestNode* buildManifestTree(char* manifest_path){
	if(manifest_path==NULL ){ pRETURN_ERROR("NULL codebook name passed", NULL); }

	//Tree to return
		ManifestNode* mantree = NULL;

	//Read file as string
		char* manifest_str = readFile(manifest_path); //reads file into a string
			if(manifest_str==NULL){ pRETURN_ERROR("error reading .Manifest", NULL); }

	//edge case: if no tokens in codebook
		if(strlen(manifest_str) == 2 ){ //TODO
				return createManifestNode("empty",-1,"empty",-1);
		}

	//LOOPING THROUGH CODEBOOK AND ADDING TO TREE
		char* curr_token = strtok( manifest_str , "\n\t"); //get rid of new line
		int mver_num = atoi( curr_token );

		char* file_name;
		int fver_num;
		char* hash;

		while( curr_token != NULL){
			//Get File_name
				curr_token = strtok( NULL, "\n\t");
					if( curr_token == NULL ) break;
					file_name = copyString( curr_token );

			//get file v_num
				curr_token =  strtok( NULL , "\n\t");
					if( curr_token == NULL ) pRETURN_ERROR("Incorrect .Manifest Format", NULL);
				fver_num = atoi( curr_token );
					if( fver_num <= 0 ) pRETURN_ERROR("not a valid version number in .Manifest file", NULL);

			//get hash
				curr_token = strtok( NULL , "\n\t");
					if( curr_token == NULL ) pRETURN_ERROR("Incorrect .Manifest Format", NULL);
				hash = copyString( curr_token );

			//insert to tree
				mantree = insertManifestTree( mantree, file_name, fver_num, hash, mver_num);
					if( mantree == NULL ) pRETURN_ERROR("ManTree", NULL);
	}

	free(manifest_str);
	return mantree;
}



/**
Searches through ManifestNode based on the key given, returns the tok/encoding associated with the key
@params: ManifestNode* root: root of tree
		 char* key: string to search for in tree
@returns: Node looking for with key (file)
 returns NULL if error
**/
ManifestNode* searchManifestTree( ManifestNode* root, char* key){
	if(root == NULL){ pRETURN_ERROR("tried to pass in NULL tree", NULL); }

	ManifestNode* ptr = root;
	while(ptr!=NULL){
		//compare key to ptr's key (ptr's key is determined by comparison mode)
		int cmp_key =  strcmp(key, ptr->file_name);

		if( cmp_key == 0 ){ //found key
			return ptr;
		}else if( cmp_key > 0 ){ //if key>ptr's key, go left
			ptr = ptr->right;
		}else if ( cmp_key < 0 ){ //if key<ptr's key, go left
			ptr = ptr->left;
		}
	}

	return NULL; //not found
}





//////Linked List Methods////////////////////////////////////////////////////////////////


//initializes head and adds node to start of linked list
bool addProjectNodePN( ProjectNode* head, char* proj_name){
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
bool delProjectNodePN(ProjectNode* head, char* proj_name){
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
ProjectNode* searchProjectNodePN(ProjectNode* head, char* proj_name){
	ProjectNode *temp = head;
	while(temp!=NULL){
		if(strcmp(temp->project_name,proj_name)==0)
			return temp;;
	}
	return NULL;
}




//PRINT METHODS/////////////////////////////////////////////////////////////
//Manifest Tree
void printManifestTree(ManifestNode* root){
	if(root==NULL){
		printf("NULL\n");
		return;
	}

	printf("---------------------------------------------\n");
	printf("[ManifestTREE:(horizontal)]\n");
	printManifestTreeRec(root,0);
	printf("---------------------------------------------\n");
}


//[private method] prints Tree recursively (horizontally). Root on far left. In order Traversal.
void printManifestTreeRec(ManifestNode* root, int space){
  	if (root == NULL)return;

  	int count = 10;
  	int i;
  	space += count; //increases space inbetween elements

    printManifestTreeRec(root->right, space);

    printf("\n");
    for (i = count; i < space; i++){ printf(" ");}
   // printf("file:%s  fv:%d  hash:%s  mv:%d\n", root->file_name, root->fver_num, root->hash,  root->mver_num);
	 printf("%s\n",root->file_name );

    printManifestTreeRec(root->left, space);
}
