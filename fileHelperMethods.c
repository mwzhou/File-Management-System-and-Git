/*
fileHelperMethods.c is a self-made file library since we're not allowed to use f-commands
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/sha.h>
#include"fileHelperMethods.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>


//FILE methods/////////////////////////////////////////////////////////////////////

/**
Goes through project sent and replaces hash in .Manifest, taking in Project Name and File Name. returns true only if file hash codes have changed
**/
bool replaceHash(char* manifest_path, FILE* commitFile, char* proj_name){

	//opening manifest file and creating file that will overwrite manifest file
	FILE * fPtr;
   	FILE * fTemp;
	fPtr = fopen(manifest_path,"r");
	if (fPtr == NULL) pEXIT_ERROR("fopen");
	
	char* temp_path = combinedPath(proj_name,"replace.tmp");
	fTemp = fopen(temp_path,"w");
	if (fTemp == NULL) pEXIT_ERROR("fopen");

	//setting initial variables	
	int lineSize = 1024;
	char buffer[lineSize];
	char temp[lineSize];
	//going through file line by line
	while((fgets(buffer, lineSize, fPtr) )!=NULL){

		if(strlen(buffer)<SHA256_DIGEST_LENGTH*2)
			continue;

		//get complete path of file
		char* end = strstr(buffer,"\t");
		int index = end-buffer;
		char* file_path = substr(buffer, 0, index+1);

		//find index of original hash in line(stored in buffer)
		int times_tab = 0;
		int vNum = 0;
		while(times_tab!=2){
			if(buffer[index] == '\t'){
				times_tab++;
				if(times_tab==1){
					char* num = substr(buffer, index+1, 2);
					vNum = atoi(num);
					free(num);
				}
			}
			index++;
		}

		//increment version number
		vNum++;

		//comparing old and new Hash's
		char* og_hash = substr(buffer, index, SHA256_DIGEST_LENGTH*2+1);
		char* new_hash = generateHash(file_path);

		//if new_hash not equal to og_hash enter into .Commit file(with v num incremented)
		if(strcmp(og_hash,new_hash)!=0){
			int len = (int)((ceil(log10(vNum))+1)*sizeof(char));
			char str[len];
			sprintf(str, "%d", vNum);

			//putting into .Commit
			fputs(file_path, commitFile);
			fputs("\t", commitFile);
			fputs(str, commitFile);
			fputs("\t", commitFile);
			fputs(og_hash, commitFile);
			fputs("\n", commitFile);
			
		}

		//entering changed hash into future replacement manifest file
		strcpy(temp, buffer);
		buffer[index] = '\0';
		strcat(buffer, new_hash);
		strcat(buffer, temp+index+strlen(new_hash));
		fputs(buffer, fTemp);

		//freeing
		free(new_hash);
		free(og_hash);
		free(file_path);
	}
	
	//replacing mnifest file with updated manifest file
	remove(manifest_path);
	rename(temp_path,manifest_path);
	
	//Fclosing
	fclose(fPtr);
   	fclose(fTemp);

	return true;
}

/**
goes through file line by line and returns the line_num w/ instance of target
**/
int extractLine(char* fpath, char* target){
    //Vars
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen( fpath , "r");
        if (fp == NULL) pEXIT_ERROR("fopen");
    
    int line_num = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        if( strstr(line, target) != NULL){ free(line); return line_num; }
        line_num++;
    }

    fclose(fp);
    if(line) free(line);
    return -1;
}

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
To be used to keep track of paths
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
To be used in fileCompressor.c recurse() to keep track of paths
Combines a path name with a file name and returns the new path
@returns: a copy of the new path
returns: NULL if invalid, non-urgent issue
**/
char* concatString(char* s1, char* s2){
	if(s1==NULL || s2==NULL){ pRETURN_ERROR("cannot pass in NULL strings", NULL); }

	//reallocate enough space
	char* ret = (char*)malloc( 1 + strlen(s1) + strlen(s2) );
		if(ret==NULL){ pEXIT_ERROR("malloc"); }

	//copies and concatenates string
	strcpy(ret, s1);
	strcat(ret, s2);

	return ret;
}

/**
mallocs copy of string
**/
char* copyString( char* s1 ){
	if( s1 == NULL ) return NULL;

	char* cpy = (char*)malloc( strlen(s1) + 1 );
	strcpy( cpy, s1);
	return cpy;
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



/**
returns true if file name ends in .hcz
**/
bool endsWithTGZ(char* file_name){
	if(file_name==NULL){ pRETURN_ERROR("null string",false);}
	int len = strlen(file_name);
	if(len<4) //minimum length is 4, i.e.: ".tgz""
		return false;

	if( file_name[len-4]=='.' && file_name[len-3]=='t' && file_name[len-2]=='g' && file_name[len-1]=='z' ) //ends with .hcz
		return true;

	return false;
}


//WRITING AND READING TO SOCKET///////////////////////////////////////////////////////

/*
sends signal to socket
*/
bool sendSig( int sockfd, bool err_cmp){
	if( err_cmp ){
		sendNumSocket( sockfd, -1);
		return false;
	}else{
		sendNumSocket( sockfd, SUCCESS_SEND);
		return true;
	}
}


/*
recieves signal from socket
*/
bool receiveSig( int sockfd ){
	int num;
	if( read(sockfd, &num,  4) < 0 ) pRETURN_ERROR("write()", false);
	return (num== SUCCESS_SEND)? true: false;
}


/**
send number to socket
**/
bool sendNumSocket( int sockfd, int num ){
	int num_send = num;
	if( write(sockfd, &num_send,  4) < 0 ) pRETURN_ERROR("write()", false);
	return true;
}



/**
sends string to socket
**/
bool sendStringSocketst( int sockfd, char* str, char* sock_type ){

	//printf("enters\n");

	//send num of bytes

	int send_bytes = strlen(str);
	if( write(sockfd, &send_bytes,  4) < 0 ) pRETURN_ERROR("write()", false);
	printf("\tsent %d number of bytes to %s\n",send_bytes, sock_type);

	//printf("bytes sent: %d\n", send_bytes);

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
			if(num_bytes<=0){ printf("\n\tError on %s side recieving string\n",  sock_type); return NULL; }
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

	/*SEND file_name*/
	if ( sendStringSocketst(sockfd, file_name, sock_type) == false ){ return false; }

	/*Open file*/
		int send_file = open(file_name, O_RDONLY);
			if(send_file<0){pRETURN_ERROR("open", false);}

	/* Sending file size */
		int file_size = sizeOfFile(file_name);
			if(file_size<0) pRETURN_ERROR("size", false);
		//write file size
		if( write(sockfd, &file_size,  4) < 0 ) pRETURN_ERROR("write()", false);
		printf("\tsent %d bytes for the file_size\n", file_size);

	/*Sending File*/
		off_t offset = 0;
		int sent_bytes = 0;
		int remain_data = file_size;
		/* Sending file data */
		while ( (remain_data > 0) && ((sent_bytes = sendfile(sockfd, send_file , &offset, 1024)) > 0) ){
					remain_data -= sent_bytes;
		}

	close(send_file);
	return true;
}



/**

recieves file for socket and writes it
**/
char* recieveFileSocketst( int sockfd, char* dir_to_store , char* sock_type ){
	/*RECIEVE FILE name*/
		char* file_name = recieveStringSocketst(sockfd, sock_type);
			if( file_name == NULL ){ pRETURN_ERROR( "Failed to recieve file name", NULL);}

		//Isolate tha file_name (no /)
			int  ind_dash = lengthBeforeLastOccChar(file_name, '/');
				if( ind_dash != -1) file_name += (ind_dash+1);

	/*path to write to*/
		char* f_path = combinedPath(dir_to_store, file_name);

	//recieve FILE
		FILE* received_file = fopen( f_path , "wb");
			if (received_file == NULL){ printf("\n\tf_path: %s\n", f_path); free(f_path);  pRETURN_ERROR( "Failed to open file", NULL); }

		//recieve file_size
		int file_size;
		READ_AND_CHECKn(sockfd, &file_size, 4);
			if( file_size<=0){ printf("\n\tError on %s side recieving file\n",  sock_type); return NULL; }
		printf("\tRecieved %d num_bytes of file_size to read from %s\n", file_size,  sock_type);

		int len;
		char buffer[1024];
		int remain_data = file_size;
		while ( ((len = recv(sockfd, buffer, 1024, 0)) > 0) && (remain_data > 0) ){
						 fwrite(buffer, 1, len, received_file);
						 remain_data -= len;
		}

	fclose(received_file);
	return f_path;
}



//Tar Methods///////////////////////////////////////////////////////////////////////

/**
tar a file and send it to socket.Update
**/
bool sendTarFilest( int sockfd, char* file_path, char* dir_to_store, char* sock_type ){
	char* tar_fp =  makeTar( file_path, dir_to_store );
		if(tar_fp==NULL){ return false; }

	if( sendFileSocketst( sockfd, tar_fp, sock_type ) == false){ return false; }

	remove( tar_fp );
	free(tar_fp);
	return true;
}


/**
**/
char* recieveTarFilest( int sockfd, char* dir_to_store , char* sock_type){
	char* tar_filepath = recieveFileSocketst(sockfd, dir_to_store, sock_type);
		if( tar_filepath == NULL) return NULL;


	char* filepath = unTar( tar_filepath );
	free( tar_filepath );
		if( filepath == NULL ) return NULL;

	return filepath;
}


/**
untars file and removes tgz file from file_directory
returns name of untarred file
**/
char* unTar( char* tar_filepath ){
	//tar -xzf .Manifest.tgz
	if(endsWithTGZ(tar_filepath)==false ){ pRETURN_ERROR("doesn't end in tgz",NULL); }

	/*Isolate file  directory*/
	char* file_dir;
	int ind_slash = lengthBeforeLastOccChar( tar_filepath , '/'); //number of chars before the last '/'

	if(ind_slash == -1) //if no /
		file_dir = "./";
	else
		file_dir = substr( tar_filepath, 0 , ind_slash+1 );

	/*get system command*/
	char* sys_cmd = (char*)malloc( strlen("tar -C ") + strlen(file_dir) + strlen(" -xzf ")+ strlen(tar_filepath)+1);
		strcpy( sys_cmd, "tar -C ");
		strcat( sys_cmd, file_dir);
		strcat( sys_cmd, " -xzf ");
		strcat( sys_cmd, tar_filepath);
		//free
			if(ind_slash != -1) free(file_dir);

	//system call
		if( system(sys_cmd) < 0){ pRETURN_ERROR("system call", NULL); }
		//free
		free( sys_cmd );

	//remove tar file
		remove( tar_filepath );
		return substr( tar_filepath, 0 , strlen(tar_filepath)-3);
}


/**
Tars a project given and returns the path of the tarred file
@params: dir_to_store: path to store tar file in
				 file_path : path of file to tar (if no path, will assume it's in the root directory)
**/
char* makeTar(char* file_path, char* dir_to_store){
	if( dir_to_store ==NULL || file_path ==NULL || typeOfFile(dir_to_store)!= isDIR || typeOfFile(file_path) == isUNDEF ){ pRETURN_ERROR("entered in invalid arguments", NULL); }

	//DECLARE VARIABLES
		//get the root directory's real path
		char* root_dir = realpath("./",NULL);
			if(root_dir==NULL){ pRETURN_ERROR("realpath", NULL); }

		//get project's real path
		char* dir_to_store_rp = realpath(dir_to_store , NULL);
			if( dir_to_store_rp == NULL){ pRETURN_ERROR("realpath", NULL); }

		char* file_dir;
		char* file_name;
		char* tar_file_path;
		char* sys_cmd; //tar cfz <dir_to_store>/<file_name>.tgz <file_name>


//GETTING THE SYSTEM COMMAND
	//GETTING FILE NAME AND FILE DIR
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
			if( chdir(file_dir) < 0 ){ free(file_dir); free(root_dir); free(dir_to_store_rp); pRETURN_ERROR("did not pass in valid file path", NULL); }
			//free
			free( file_dir );
		}


	//gets path of tar file
		tar_file_path =  (char*)malloc( strlen(dir_to_store_rp) + strlen(file_name) + strlen(".tgz") + 2);
			//cpy info
			strcpy( tar_file_path, dir_to_store_rp);
			strcat( tar_file_path, "/");
			strcat( tar_file_path, file_name);
			strcat( tar_file_path, ".tgz");
			//free
			free(dir_to_store_rp);

	//constructing sys_cmd
		sys_cmd =  (char*)malloc(strlen("tar cfz ") + strlen(tar_file_path) + 1 + strlen(file_name) + 1 );
			//cpy info
			strcpy( sys_cmd, "tar cfz ");
			strcat( sys_cmd, tar_file_path);
			strcat( sys_cmd, " ");
			strcat( sys_cmd, file_name );

	//RUN SYSTEM COMMAND
		if( system(sys_cmd)< 0 ){free( sys_cmd ); free( root_dir ); pRETURN_ERROR("system", NULL); }
			//free
			free( sys_cmd );


//change back to root directory
	if( chdir(root_dir) < 0 ){ free( root_dir ); pRETURN_ERROR("changing root directory", NULL); }


//free and return
free( root_dir );
return tar_file_path;

}



////////////////////////////////////////////////////////////////////////

/**
creates manifest file of a whole project, returns path of file (malloced)
**/
char* createManifest(char* proj_name){
	//create manifest file
	char* manifest_path = combinedPath(proj_name, ".Manifest");
	int manifest_fd = openFileW( manifest_path );
		if( manifest_fd < 0){ pRETURN_ERROR("open", NULL); }

	WRITE_AND_CHECKb( manifest_fd, "1\n", 2);

	//write, if failed, remove file and return false
	if( writeToManifest( proj_name, manifest_fd ) == false ){
		REMOVE_AND_CHECK(manifest_path);
		free(manifest_path);
		close(manifest_fd);
		return NULL;
	}

	WRITE_AND_CHECKb( manifest_fd, "\n", 1);

	close(manifest_fd);
	return manifest_path;
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
		if(fp < 0) { pRETURN_ERROR("fopen", NULL);}

	//Updatting hash code
	while ((bytes = read(fp,buffer, file_size))){
		SHA256_Update(&ctx, buffer, bytes);
	}
	SHA256_Final(hash, &ctx);

	char* output = (char*)malloc( SHA256_DIGEST_LENGTH*2 + 1 );

	output[ SHA256_DIGEST_LENGTH *2] = '\0';

	int i;
	for(i=0; i<SHA256_DIGEST_LENGTH; i++){
		sprintf(output+(i*2), "%02x", hash[i]);
	}
	free(buffer);

	//returning hashcode generated
	return output;
}
