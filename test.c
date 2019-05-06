#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<errno.h>
#include <openssl/sha.h>
#include<math.h>


#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<sys/types.h>

#include"fileHelperMethods.h"



int main(int argc, char * argv[]){

	char* buffer2 = "Asst1/mymalloc.c	U	1	c5b06b94a970266c97c21bcbda719b3274ba4a86cce787ce8be9ea5dcc7bb2e1\n";
	char* buffer = "Asst1/mymalloc.c	1	c5b06b94a970266c97c21bcbda719b3274ba4a86cce787ce8be9ea5dcc7bb2e1\n";
	char* toUpdate = "Asst1/mymalloc.c	3	jv7872nf87n4f87nu8907h4fn38c94787nbvc48387nbcv859738vn7897588747\nAsst1/mymaffoc.c	4	jv4556nf87n4f87nu8907h4fn38c94787nbvc48387nbcv859738vn7897588747\n";

	//char buffer2[lineSize];
	//while((fgets(buffer2, lineSize, cmP) )!=NULL){
		char* start = strstr(buffer2,"\t");
		int index = start-buffer2;
		char* command = substr(buffer2,index+1,2);
		if(strcmp(command, "U")==0){
			
			//finding fileName
			char* fileName = substr(buffer2,0,index+1);
			//fputs(fileName,toChangeFile);
			//fputs("\t",toChangeFile);
			printf("fileName: %s\n",fileName);

			//finding version number
			index++;
			while(buffer2[index]!='\t')
				index++;
			char* vNum = substr(buffer2,index+1,2);
			//fputs(vNum,toChangeFile);
			//fputs("\t",toChangeFile);
			printf("%s\n",vNum);

			//finding hash
			index++;
			while(buffer2[index]!='\t')
				index++;
			char* hash = substr(buffer2,index+1,SHA256_DIGEST_LENGTH*2+1);
			//fputs(hash,toChangeFile);
			//fputs("\n",toChangeFile);

			free(hash);
			free(vNum);
			free(fileName);
		}
		free(command);
	//}




	//while((fgets(buffer, lineSize, cmP) )!=NULL){
	
		//find file name
		start = strstr(buffer,"\t");
		int index5 = start-buffer;
		char* fileNameMan = substr(buffer,0,index5+1);

		//if manifest file name is not in toUpdate file, copy into manifest
		char* inUpdate = strstr(toUpdate, fileNameMan);
		if(inUpdate==NULL){
			printf("enters!");
			//fputs(buffer,tempFile);
		}
		//if it is in toUpdate file, replace current line with what is in toUpdate
		else{
			int index2 = inUpdate - toUpdate;
			int index3 = index2;
			while(toUpdate[index3]!='\n')
				index3++;
			printf("%d\t%d\n",index2,index3);
			char* enter = substr(toUpdate, index2, index3-index2+1);
			printf("%s\n",enter);
			//fputs(enter,tempFile);
			free(enter);
		}
		free(fileNameMan);
	//}

	return 0;

}
