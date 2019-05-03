#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<errno.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<sys/types.h>

#include"fileHelperMethods.h"



void add( char* proj_name , char* file_name ){

	//Get paths of manifest file and file to write into manifest file
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	int manifest_fd = open( manifest_path, O_WRONLY|O_APPEND, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) ); //writing in manifest file
		if(manifest_fd < 0){ fprintf( stderr, "file:%s\n",file_name ); pRETURN_ERRORvoid("tried to open file flags: (O_WRONLY|O_APPEND)"); }

	//if(manifest_fd<0) { pRETURN_ERRORvoid("Error on opening file"); }

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

}


int main(int argc, char * argv[]){

	//createManifest("Asst1");

	//add("Asst1","happy.c");
	createManifest("r1");
	char* m_p = combinedPath( "r1" , ".Manifest");
	ManifestNode* m =  buildManifestTree( m_p );
	printManifestTree(m);

	free(m_p);
	freeManifestTree(m);

	return 0;

}
