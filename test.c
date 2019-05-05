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

	char* buffer = "Asst1/memgrind.c	1	894307907589789478378074389078";
	//char* proj_name = "Asst1";

	char* end = strstr(buffer,"\t");
	int index_end = end-buffer;
	char* start = strstr(buffer,"/");
	int index_start = start-buffer;
	printf("%d\t%d\t%d\n",index_start, index_end, (index_end-index_start)+1);
	char* file_path = substr(buffer, index_start+1, (index_end-index_start)); 
	printf("%s\n",file_path);

	/*char* systemCall = concatString("cp ",proj_name);
	systemCall = concatString(concatString, " ");
	systemCall = concatString(concatString, copyPath);

	system(sys_cmd);*/

	/*char* dir_to_send = concatString(proj_name,".to_send");
	struct stat st = {0};
	//check if directory exists, otherwise create it
	if(stat(dir_to_send, &st) == -1){
		mkdir(dir_to_send, 0777);
	}*/



	return 0;

}
