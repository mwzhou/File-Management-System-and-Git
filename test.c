#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<errno.h>
#include<math.h>


#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<sys/types.h>

#include"fileHelperMethods.h"



int main(int argc, char * argv[]){

	char* version_num = "5";
	char* proj_name = "Asst1";

	char* fullPath = realpath("Asst1",NULL);
	int index_end = lengthBeforeLastOccChar(fullPath , '/');
	char* dir_to_store = substr(fullPath, 0, index_end+1);
	free(fullPath);
	

	//getting path of tared version
	char* versionTar = concatString("5",".tgz");
	char* back_proj = concatString("Asst1", ".bak");
	char* versionPath = combinedPath(back_proj, versionTar);

	//deleting all tar files post requestiod version number
	struct dirent *de;
	DIR *dr = opendir(back_proj);
	//if(dr==NULL){pRETURN_ERRORvoid("directory could not be opened");}

	while((de = readdir(dr))!=NULL){
		char* char_vNum = substr(de->d_name, 0, 2);
		int curr_vNum = atoi(char_vNum);
			
		//delete if tar file is a greater version num than the one requested
		if(curr_vNum > atoi(version_num)){
			char* dir_to_delete = combinedPath(back_proj, de->d_name);
			removeDir(dir_to_delete);
			free(dir_to_delete);
		}

		//replace current project with version number requested
		else if(curr_vNum == atoi(version_num)){
			char* name = unTar(versionPath);
			moveFile(name, dir_to_store);
			removeDir(proj_name);
			char* version_file_path = combinedPath(dir_to_store, version_num);
			rename(version_file_path,proj_name);
			free(name);
			free(version_file_path);
		}
		free(char_vNum);
	}

	return 0;

}
