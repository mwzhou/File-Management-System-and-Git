/*
fileHelperMethods.c is a self-made file library since we're not allowed to use f-commands
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<errno.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include<sys/types.h>
#include"fileHelperMethods.h"


//FILE methods/////////////////////////////////////////////////////////////////////

/**
returns size of file in bytes
returns -1 on error
**/
int sizeOfFile(char* file_name){
	int file = open(file_name, O_RDONLY);
		if( file < 0 ){ fprintf( stderr, "file_name: %s\n",file_name); close(file); pRETURN_ERROR("error opening file (O_RDONLY)", -1); }

	int file_len = (int)lseek( file , 0, SEEK_END ); //gets file size in bytes by going to end of file_cpy
		if ( file_len < 0 ){ close(file); pRETURN_ERROR("error getting file length with lseek()", -1); }//checking if file_len is a valid length

	close(file);
	return file_len;
}


/**
reads a file given a filename.
@returns: string of contents of file if successful
 returns NULL if unsuccessful
**/
char* readFile(char* file_name){
	//INITIALIZING VARIABLES AND OPENING THE FILE
		//Opening files
		int file = open(file_name, O_RDONLY);
			 if( file < 0 ){ fprintf( stderr, "file_name: %s\n",file_name); pRETURN_ERROR("error opening file (O_RDONLY)", NULL); }

		//Initializing file length in bytes
		int file_len = sizeOfFile(file_name); //length of file in bytes

		//Initializing File Strings to return
		char* fstr = (char*)calloc((file_len + 1), 1); //string with file's contents, return this string on success
			if( fstr == NULL ){ pEXIT_ERROR("calloc()"); }


	//READING THE FILE
	int bytes_read = read(file, fstr, file_len); //number of bytes read through read
		if(bytes_read < 0){ fprintf( stderr, "file_name: %s\n",file_name);close(file); free(fstr); pRETURN_ERROR("error reading file", NULL); }

	fstr[bytes_read] = '\0'; //mark end of string

	close(file);
	return fstr;
}


/**
opens file for writing with name (file_name)
returns file descriptor if successful
returns -1 if not
Note: does not free file_name
**/
int openFileW(char* file_name){
	if(file_name ==NULL){ pRETURN_ERROR("null file path", -1); }

	int fd = open( file_name, O_WRONLY|O_CREAT|O_TRUNC, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) ); //creates a new file, overwrites old file if exists
		if(fd < 0){ fprintf( stderr, "file:%s\n",file_name ); pRETURN_ERROR("tried to open file flags: (O_WRONLY|O_CREAT|O_TRUNC)", -1); }

	return fd;
}



/**
returns the type of the string given in
@params: char* name - file_name or path_name
@returns FileType:
	isDIR - directory
	isREG - regular file
	isUNDEF - error
**/
FileType typeOfFile(char* file_name){
	if(file_name==NULL){ pRETURN_ERROR("passed in NULL path", isUNDEF); }

	//GET REAL PATH
	char* file_rp = realpath(file_name, NULL);
		if( file_rp  == NULL){ return isUNDEF; }
		if( file_rp[(int)(strlen(file_rp)-1) ] == '~' ){ free(file_rp); return isUNDEF; }

	//GET STAT
	struct stat dpstat;
	if(lstat( file_rp , &dpstat ) < 0){ free(file_rp); return isUNDEF; } //file doesn't exist
	free(file_rp);

	//check if DIR, REG, or LINK, and returns the respective number (defined in macro)
	if(S_ISREG(dpstat.st_mode)) //directory or file
		return isREG;
	else if(S_ISDIR(dpstat.st_mode))
		return isDIR;
	else
		return isUNDEF;
}



//WRITING AND READING TO SOCKET///////////////////////////////////////////////////////

/**
send error to socket
**/
bool sendErrorSocket( int sockfd ){
	int err = -1;
	if( write(sockfd, &err,  4) < 0 ) pRETURN_ERROR("write()", false);
	return true;
}


/**
sends file to socket
**/
bool sendFile( int sockfd, char* file_contents ){
	//send num of bytes
	int send_bytes = strlen(file_contents);
	if( write(sockfd, &send_bytes,  4) < 0 ) pRETURN_ERROR("write()", false);
	if( write(sockfd, file_contents, send_bytes) < 0 ) pRETURN_ERROR("write()", false);

	return true;
}

/**
Given the path of a project/file, Scan through directory recursively (go through directories within directories as well) for every file and add to Manifest
**/
bool createManifest (char* file_Path){

	//Pointer for directory
	struct dirent *de;

	//Find path of current directory
	int ind_slash = lengthBeforeLastOccChar( file_Path , '/'); //number of chars before the last '/'
	char* currFile_dir = substr( file_Path , 0 , ind_slash+1 );
	//printf("currentPath is: %s\n",currFile_dir);

	//Opening the directory of path given
	DIR *dr = opendir(file_Path);
	if(!dr)
		return false;

	//enter directory of give file
	chdir(file_Path);
	//printf("enetring Path: %s\n",file_Path);

	//variable where hash_code will be stored
	char* hash_code;

	//reads through files and directories
	while((de = readdir(dr)) !=NULL){
		if(strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0){
			//Finding name of file and cending path back into method incase of being a directory
			char* currFile = de->d_name;
			char* path_of_file = realpath(currFile, NULL);
			//If not a directory, generates a hash for the file
			bool file_or_dir = createManifest(path_of_file);
			if(file_or_dir == false){
				//printf("hash being created for file: %s\n", currFile);
				hash_code = generateHash(currFile);
				printf("%s\n\n", hash_code);
			}
		}
	}

	//change back to parent directory to continue searching for files and free
	chdir(currFile_dir);
	free(currFile_dir);
	
	//close and return
	closedir(dr);
	return true;	
		
}

/**
Generate hash code for given file
**/

char* generateHash (char* file_name){

	//Find size offile in bytes	
	int file_size = sizeOfFile(file_name);

	char* buffer = (char*) malloc (file_size*sizeof(char));
	int bytes;

	//Object to hold state of Hash
	SHA256_CTX ctx;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_Init(&ctx);
	
	//Setting up and opening file
	int fp;
	fp = open(file_name, O_RDONLY);
	if(fp < 0) { pRETURN_ERROR("error on fopen",false);}

	//Updatting hash code
	while ((bytes = read(fp,buffer, file_size))){
		SHA256_Update(&ctx, buffer, bytes);
	}
	SHA256_Final(hash, &ctx);

	char* output = (char*)malloc(sizeof(char)*60);

	int i;
	for(i=0; i<SHA256_DIGEST_LENGTH; i++){
		sprintf(output+(i*2), "%02x", hash[i]);
	}

	//returning hashcode generated
	return output;
}

