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


int main(int argc, char * argv[]){
	
	//char* proj_name = "Asst1";

	char buffer[1024];
	strcpy(buffer,"Asst1/\n");

	printf("%d\n",(int)(strlen(buffer)));

	int index_end = lengthBeforeLastOccChar(buffer, '/');
	char* dir_to_store = substr(buffer,0,index_end+1);
	//char* filePath = recieveTarFile( sockfd, dir_to_store);

	printf("%s\n",dir_to_store);


	return 0;

}
