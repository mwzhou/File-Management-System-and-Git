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



char* getPathProjFile( char* proj_name, char* file_name ){
	return NULL;
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
sends string to socket
**/
bool sendStringSocketst( int sockfd, char* str, char* sock_type ){

	//send num of bytes
	printf("sending number of bytes to %s\n", sock_type);
	int send_bytes = strlen(str);
	if( write(sockfd, &send_bytes,  4) < 0 ) pRETURN_ERROR("write()", false);

	//sending string
	printf("sending string to %s\n", sock_type);
	if( write(sockfd, str , send_bytes) < 0 ) pRETURN_ERROR("write()", false);

	return true;
}



/**
Recieves string from socket
**/
char* recieveStringSocketst( int sockfd, char* sock_type ){
	//recieve num bytes
		int num_bytes;
		READ_AND_CHECKe(sockfd, &num_bytes, 4);
			if(num_bytes<=0){ printf("\tError on %s side\n",  sock_type); return NULL; }
		printf("\tRecieved %d num_bytes to read from %s\n", num_bytes,  sock_type);

	//recieve string contents
		char* str = (char*)malloc(num_bytes + 1);
			if(str == NULL){ pRETURN_ERROR("malloc",NULL); }
		READ_AND_CHECKe(sockfd, str, num_bytes);
		str[num_bytes] = '\0';

		printf("\tRecieved information from %s\n", sock_type);
		return str;
}
