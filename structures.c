
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include<errno.h>
#include <pthread.h>

#include "structures.h"

//////ManifestNode Linked List Methods////////////////////////////////////////////////////////////////

ManifestNode* createManifestNode(	int mver_num, char* file_name, int fver_num, char* hash){
	ManifestNode* ret = malloc(sizeof(ManifestNode));

	ret-> mver_num = mver_num;
	ret-> file_name = file_name;
	ret-> fver_num = fver_num;
	ret->hash = hash;

	ret->next=NULL;
	return ret;
}


//adds node from front
bool addManifestNode( ManifestNode** head_addr, ManifestNode* to_add ){
		if(to_add==NULL) return false;

		if(head_addr==NULL || *head_addr==NULL){
			*head_addr = to_add; //update head
		}else{
			to_add->next = *head_addr;
			*head_addr = to_add; //update head
		}
		return true;
}



//Deletes a node, returs true if found and deleted, returns false if not found
bool delManifestNode(ManifestNode** head_addr, char* file_name){
	if(file_name==NULL) return false;

	ManifestNode* head = *head_addr;
	ManifestNode* prev = NULL;
	ManifestNode* ptr = head;

	//search for string and get node
	int cmp = -1;
	while( ptr!=NULL ){
		if( (cmp = strcmp(ptr->file_name,file_name)) == 0 )break;
		prev = ptr;
		ptr = ptr->next;
	}

	//did not find node
	if(cmp == -1 ) return false;

	//delete head
	if( prev == NULL ){
		ManifestNode* temp = head;
		*head_addr = head->next; //update head

		//free
		free(temp->file_name);
		free(temp->hash);
		free(temp);

	//delete
	}else{
		ManifestNode* temp = ptr;
		prev->next = ptr->next;
		*head_addr = head;

		//free
		free(temp->file_name);
		free(temp->hash);
		free(temp);
	}

	return true;
}



//Returns node of project when given project name to find
ManifestNode* searchManifestNode(ManifestNode* head, char* file_name){
	ManifestNode* ptr = head;
	while(ptr!=NULL){
		if(strcmp(ptr->file_name,file_name)==0)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}



/**
builds Manifest Linked List
@returns Manifest LL head if successful
 returns NULL if error
 returns EMPTY NODE if manifest has no entries
**/
ManifestNode* buildManifestLL(char* manifest_path){
    if(manifest_path==NULL ){ pRETURN_ERROR("NULL codebook name passed", NULL); }

    //Tree to return
        ManifestNode* manifestLL = NULL;

    //Read file as string
        char* manifest_str = readFile(manifest_path); //reads file into a string
            if(manifest_str==NULL){ pRETURN_ERROR("error reading .Manifest", NULL); }

    //edge case: if no tokens in codebook
        if(strlen(manifest_str) == 2 ){ //TODO
            return createManifestNode(-1,"empty",-1,"empty");
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
							ManifestNode* to_add =  createManifestNode(mver_num, file_name, fver_num, hash);
							addManifestNode( &manifestLL,  to_add );
    		}

    free(manifest_str);
    return manifestLL;
}




//////Linked List Methods////////////////////////////////////////////////////////////////

//initializes head and adds node to start of linked list
bool addProjectNodePN( ProjectNode** head, char* proj_name){
	return false;
}


//Deletes a node, returs true if found and deleted, returns false if not found
bool delProjectNodePN(ProjectNode** head, char* proj_name){
	return false;
}


//Returns node of project when given project name to find
ProjectNode* searchProjectNodePN(ProjectNode* head, char* proj_name){
	return NULL;
}

//////////////////////////////////////////////////

/**
Print Manifest Node
**/
void printManifestNode( ManifestNode* head ){
	ManifestNode* ptr = head;
	while(ptr!=NULL){
		printf("mv:%-5d\tfv:%-5d\tfile: %10s\t\thash: %-20s\n", ptr->mver_num,  ptr->fver_num, ptr->file_name, ptr->hash);
		ptr = ptr->next;
	}
}