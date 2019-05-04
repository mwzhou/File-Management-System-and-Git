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

	//createManifest("Asst1");

	char* proj_name = "Asst1";	
	char* commitFile = combinedPath(proj_name,".Commit");
	//unlink(commitFile);
	char* clientManifest = combinedPath(proj_name, ".Manifest");

	FILE* newFile = fopen(commitFile, "w");
	
	replaceHash(clientManifest, newFile, "Asst1");

	

	fclose(newFile);
	free(commitFile);

	return 0;

}
