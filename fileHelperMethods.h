#ifndef FILE_HELP
#define FILE_HELP

//ERROR MACROS
	#define PRINT_ERROR(txt) ({\
				if(errno!=0)\
					fprintf(stderr, "ERROR:%s\nerrno:%s\nFile:%s  on Line:%d  in Func:%s()\n\n", txt, strerror(errno), __FILE__, __LINE__, __func__);\
				else\
					fprintf(stderr, "ERROR:%s\nFile:%s  on Line:%d  in Func:%s()\n\n", txt, __FILE__, __LINE__, __func__);\
			})


	#define pRETURN_ERROR(txt,ret_val) do{  PRINT_ERROR(txt); return ret_val; }while(0)
	#define pRETURN_ERRORvoid(txt) do{  PRINT_ERROR(txt); return; }while(0)

	#define pEXIT_ERROR(txt) do{ PRINT_ERROR(txt); exit(EXIT_FAILURE); }while(0)


//MACROS FOR READABILITY
	#define WRITE_AND_CHECKe(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pEXIT_ERROR("write()"); } }while(0) //writes to file, if failed, prints out error and returns void
	#define WRITE_AND_CHECKn(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pRETURN_ERROR("write()", NULL); } }while(0) //writes to file, if failed, prints out error and returns void

	#define READ_AND_CHECKe(file, buf, nbytes) do{ if( read(file, buf , nbytes)<0 ) pEXIT_ERROR("read()"); }while(0)
	#define READ_AND_CHECKn(file, buf, nbytes) do{ if( read(file, buf , nbytes)<0 ) pRETURN_ERROR("read()", NULL); }while(0)

	#define REMOVE_AND_CHECK(file_name) do{ if(remove(file_name) == 0) fprintf( stderr, "removed file:%s\n",file_name); else fprintf( stderr, "couldn't remove file:%s",file_name);  }while(0) //removes file and prints if successful


//ENUMS
	//enum to differentiate between FileTypes
	typedef enum{ isDIR=017, isREG=736, isUNDEF=-1 }FileType;


//METHOD SIGNATURES
	int sizeOfFile(char* file_name);
	char* readFile(char* file_name);
	int openFileW(char* file_name);
	FileType typeOfFile(char* file_name);

	bool sendErrorSocket( int sockfd );
	bool sendFile( int sockfd, char* file_contents );

	char* generateHash (char* file_name);
	bool createManifest (char* file_Path);

#endif
