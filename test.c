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
	//printf("%s\n", readFile("./r1/.Commit"));
	//printf("%d\t%d\n", typeOfFile("./trial"), typeOfFile("misc") );

	char* match = findFileMatchInDir("trial", "CommitClient");
	printf("%s\n", match);
	free(match);
	return 0;
}
