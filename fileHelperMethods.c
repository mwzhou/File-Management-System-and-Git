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
#include <openssl/sha.h>
#include"fileHelperMethods.h"

#include <sys/types.h>
#include <sys/socket.h>


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

	if( file_name[(int)(strlen(file_name)-1)] == '~' ){ return isUNDEF; }
	if( file_name  == NULL){ return isUNDEF; }

	//GET STAT
	struct stat dpstat;
	if(lstat( file_name , &dpstat ) < 0){ return isUNDEF; } //file doesn't exist

	//check if DIR, REG, or LINK, and returns the respective number (defined in macro)
	if(S_ISREG(dpstat.st_mode)) //directory or file
		return isREG;
	else if(S_ISDIR(dpstat.st_mode))
		return isDIR;
	else
		return isUNDEF;
}



//STRING MANIPULATION methods/////////////////////////////////////////////////////////////////////

/**
To be used in fileCompressor.c for decompress.
returns a subtring of s from the start_index to start_index+length of substring
returns NULL if start_ind+length-1 > strlen(s) or could not get a substring
**/
char* substr(char* s, size_t start_ind, size_t length){
	if( s==NULL||start_ind<0||length<0 ){ pRETURN_ERROR("faulty parameters", NULL); }
	if( (start_ind+length-1)  > strlen(s) ){ pRETURN_ERROR("start_ind+length-1 cannot be larger than the string passed in",NULL); }

	char* ret = (char*)malloc(length); //malloc string to return
		if(ret==NULL){ pEXIT_ERROR("malloc"); }

	memcpy(ret, s+start_ind, length); //copies s+start to length into ret
	ret[length - 1] = '\0';

	return ret;
}


/**
To be used in fileCompressor.c recurse() to keep track of paths
Combines a path name with a file name and returns the new path
@returns: a copy of the new path
returns: NULL if invalid, non-urgent issue
**/
char* combinedPath(char* path_name, char* file_name){
	if(path_name==NULL || file_name==NULL){ pRETURN_ERROR("cannot pass in NULL string into combinedPath()", NULL); }

	//reallocate enough space
	char* ret = (char*)malloc( 2 + strlen(path_name) + strlen(file_name) );
		if(ret==NULL){ pEXIT_ERROR("malloc"); }

	//copies and concatenates string
	strcpy(ret, path_name);
	strcat(ret, "/");
	strcat(ret, file_name);

	return ret;
}

/**
returns number of characters in s before the last occurrence of c
**/
int lengthBeforeLastOccChar( char* s, char c){
	//FINDING NUMBER OF CHARACTERS before c if it exists, if no occurrence, it returns -1
		int len = strlen(s);
		int i;
		for(i = len; i>=0; i--){
			if ( s[i] == c){
				return i;
			}
		}

		return -1;
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
	printf("\tsending number of bytes to %s\n", sock_type);
	int send_bytes = strlen(str);
	if( write(sockfd, &send_bytes,  4) < 0 ) pRETURN_ERROR("write()", false);

	//sending string
	printf("\tsending string to %s\n", sock_type);
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

		return str;
}


/**
note: file_name must just be the project_name and the file_name, not the path
**/
bool sendFileSocketst( int sockfd, char* file_name, char* sock_type ){
	//SEND file_name
	if ( sendStringSocketst(sockfd, file_name, sock_type) == false ){ return false; }

	//get size of file
	int size_file = sizeOfFile(file_name);
		if(size_file <0){ sendErrorSocket(sockfd);  pRETURN_ERROR("retrieving size of file",false);}
	//write size of file to socket
	printf("\tSending size of file to %s\n", sock_type);
	if( write(sockfd, &size_file ,  4) < 0 )  pRETURN_ERROR("write()", false);


	//Open file
	FILE* fp = fopen(file_name ,"rb");

	//writes to file
	int len;
	char buf[1024] = {0};
	printf("\tWriting file to %s\n", sock_type);
	while( (len = fread(buf, 1, 1024, fp)) > 0  ){
		if( send(sockfd ,buf,strlen(buf),0) < 0 ) { printf("Error sending file to %s\n", sock_type); }
	}

	fclose( fp );
	return true;
}


/**
**/
char* recieveFileSocketst( int sockfd, bool modify_fname, char* sock_type ){
	//RECIEVE FILE name
		char* file_name = recieveStringSocketst(sockfd, sock_type);
		char* ret_path = NULL;

		//path to write to
			//if modify file_name
			if(modify_fname){
				ret_path = (char*)malloc( strlen(file_name) + strlen(sock_type) + 2 );
					strcpy( ret_path , sock_type );
					strcat( ret_path, "." );
					strcat( ret_path, file_name );
					free(file_name);
			//don't modify file_name
			}else{
				ret_path = file_name;
			}


	//recieve num bytes OF FILE
		int num_bytes;
		READ_AND_CHECKe(sockfd, &num_bytes, 4);
			if(num_bytes<0){ free(ret_path);  printf("\tError from %s side reading num_bytes of file\n",  sock_type); return NULL; }

	//recieve FILE
		FILE* received_file = fopen( ret_path , "wb");
			if (received_file == NULL){ free(ret_path); pRETURN_ERROR( "Failed to open file", NULL); }

	//Write info to file
		ssize_t len;
		char buf[1024] = {0};
		while ( (len = recv( sockfd ,buf, 1024 ,0)) > 0 ){
					buf[len] = 0;
	        if ( (fwrite(buf,1,strlen(buf), received_file)) < 0){ pRETURN_ERROR("writing", NULL); }
		}
		printf("\tRecieved file from %s\n", sock_type);

	fclose(received_file);
	return ret_path;
}



//Tar Methods///////////////////////////////////////////////////////////////////////

bool sendTarFile(){
	return false;
}

/**
Tars a project given and returns the path of the tarred file
@params: proj_name : project of where to put the file (in its backup folder)
				 file_path : path of file to tar (if no path, will assume it's in the root directory)
**/
/*
char* makeTar(char* file_name, char* path_to_store){
	if( proj_name==NULL || file_name ==NULL || typeOfFile(path_to_store)!= isDIR || typeOfFile(file_name) == isUNDEF ){ pRETURN_ERROR("entered in invalid arguments", NULL); }

	//DECLARE VARIABLES
		//get the root directory's real path
		char* root_dir = realpath("./",NULL);
			if(root_dir==NULL){ pRETURN_ERROR("realpath", NULL); }
		//get project's real path
		char* proj_rp = realpath(proj_name,NULL);
			if( proj_rp ==NULL){ pRETURN_ERROR("realpath", NULL); }

		char* file_dir;
		char* file_name;
		char* proj_bak_name;
		char* tar_file_path;
		char* sys_cmd; //tar cfz <proj_name_path>.bak/<file_name>.tgz <file_name>


//GETTING THE SYSTEM COMMEND
	//find out if there is a '/' in file_name, if not, it will assume that the file is in the root dir
		int ind_slash = lengthBeforeLastOccChar( file_path , '/'); //number of chars before the last '/'

		//file is in root directory
		if(ind_slash == -1){
			file_name = file_path;

		//file is not in the root directory - isolate the file_directory and the file_name
		}else{
			//isolating the file_directory and the file_name
			file_dir = substr( file_path , 0 , ind_slash+1 );
			file_name =  file_path+(ind_slash+1); //Note: NOT malloced
			//change the directory to where the file is
			if( chdir(file_dir) < 0 ){ free(file_dir); free(root_dir); free(proj_rp); pRETURN_ERROR("did not pass in valid file path", NULL); }
			//free
			free( file_dir );
		}


	//get path of the backup_project directory
		proj_bak_name = (char*)malloc( strlen(proj_rp) + 6 );
			//cpy info
			strcpy( proj_bak_name, proj_rp);
			strcat( proj_bak_name, ".bak/");
			//free
			free(proj_rp);


	//gets path of tar file
		tar_file_path =  (char*)malloc( strlen(proj_bak_name) + strlen(file_name) + strlen(".tgz") + 1);
			//cpy info
			strcpy( tar_file_path, proj_bak_name);
			strcat( tar_file_path, file_name);
			strcat( tar_file_path, ".tgz");
			//free
			free( proj_bak_name );


	//constructing sys_cmd
		sys_cmd =  (char*)malloc(strlen("tar cfz ") + strlen(tar_file_path) + 1 + strlen(file_name) + 1 );
			//cpy info
			strcpy( sys_cmd, "tar cfz ");
			strcat( sys_cmd, tar_file_path);
			strcat( sys_cmd, " ");
			strcat( sys_cmd, file_name );

	//RUN SYSTEM COMMAND
		system(sys_cmd);
			//free
			free( sys_cmd );


//change back to root directory
	if( chdir(root_dir) < 0 ){ free( root_dir ); pRETURN_ERROR("changing root directory", NULL); }


//free and return
free( root_dir );
return tar_file_path;

}
*/
////////////////////////////////////////////////////////////////////////


/**
**/
bool createManifest(char* proj_name){
	//create manifest file
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	int manifest_fd = openFileW( manifest_path );

	//write, if failed, remove file and return false
	if( writeToManifest( proj_name, manifest_fd ) == false ){
		REMOVE_AND_CHECK(manifest_path);
		free(manifest_path);
		close(manifest_fd);
		return false;
	}

	free(manifest_path);
	close(manifest_fd);
	return true;
}



/**
Given the path of a project, Scan through directory recursively (go through directories within directories as well) for every file and add to Manifest
**/
bool writeToManifest(char* path, int  manifest_fd ){
	//Pointer for directory
	struct dirent *de;

	//Opening the directory of path given
	DIR *dr = opendir(path);
		if(!dr) pRETURN_ERROR("not a directory", false);


	//reads through files and directories
	while((de = readdir(dr)) !=NULL){
		if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0){ continue; }

			//Finding name of file and cending path back into method incase of being a directory
			char* new_path = combinedPath(path, de->d_name);
			int np_type = typeOfFile(new_path);


			if( np_type  == isDIR ){
					//recurse
					writeToManifest( new_path , manifest_fd);

			}else if( np_type == isREG ){
						char* hash_code = generateHash(new_path);
						//printf("path:%s\t%s\t\t%s\t%d\n", new_path, path, de->d_name, np_type);
						//printf("%s\thash:%s\n", new_path, hash_code);

						//Write to manifest fd
						WRITE_AND_CHECKb( manifest_fd, new_path, strlen(new_path));
						WRITE_AND_CHECKb( manifest_fd, "\t", 1);
						WRITE_AND_CHECKb( manifest_fd, "1", 1);
						WRITE_AND_CHECKb( manifest_fd, "\t", 1);
						WRITE_AND_CHECKb( manifest_fd, hash_code, strlen(hash_code));
						WRITE_AND_CHECKb( manifest_fd, "\n", 1);

						free(hash_code);
			}

			free(new_path);
		}

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

	char* buffer = (char*) malloc (file_size);
	int bytes;

	//Object to hold state of Hash
	SHA256_CTX ctx;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_Init(&ctx);

	//Setting up and opening file
	int fp = open(file_name, O_RDONLY);
		if(fp < 0) { pRETURN_ERROR("error on fopen", NULL);}

	//Updatting hash code
	while ((bytes = read(fp,buffer, file_size))){
		SHA256_Update(&ctx, buffer, bytes);
	}
	SHA256_Final(hash, &ctx);

	char* output = (char*)malloc( SHA256_DIGEST_LENGTH + 1 );

	int i;
	for(i=0; i<SHA256_DIGEST_LENGTH; i++){
		sprintf(output+(i*2), "%02x", hash[i]);
	}
	//KEEP HERE BECAUSE SPRINTF
	output[ SHA256_DIGEST_LENGTH ] = '\0';
	free(buffer);

	//returning hashcode generated
	return output;
}


////////////////////////////////////////////////////////////////////////
