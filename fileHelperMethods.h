#ifndef FILE_HELP
#define FILE_HELP

//ERROR MACROS
	#define PRINT_ERROR(txt) ({\
				if(errno!=0)\
					fprintf(stderr, "ERROR:%s  errno:%s\nFile:%s  on Line:%d  in Func:%s()\n", txt, strerror(errno), __FILE__, __LINE__, __func__);\
				else\
					fprintf(stderr, "ERROR:%s\nFile:%s  on Line:%d  in Func:%s()\n", txt, __FILE__, __LINE__, __func__);\
			})
			
			
	#define pRETURN_ERROR(txt,ret_val) do{  PRINT_ERROR(txt); return ret_val; }while(0)
	#define pRETURN_ERRORvoid(txt) do{  PRINT_ERROR(txt); return; }while(0)
	#define pEXIT_ERROR(txt) do{  PRINT_ERROR(txt); exit(EXIT_FAILURE); }while(0)

//MACROS FOR READABILITY
	#define WRITE_AND_CHECKf(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pRETURN_ERROR("write()", false); } }while(0) //writes to file, if failed, prints out error and returns void
	#define REMOVE_AND_CHECK(file_name) do{ if(remove(file_name) == 0) fprintf( stderr, "removed file:%s\n",file_name); else fprintf( stderr, "couldn't remove file:%s",file_name);  }while(0) //removes file and prints if successful


//ENUMS
	//enum to differentiate between FileTypes
	typedef enum{ isDIR=017, isREG=736, isUNDEF=-1 }FileType;


//METHOD SIGNATURES
	int sizeOfFile(char* file_name);
	char* readFile(char* file_name);
	int openFileW(char* file_name);
	FileType typeOfFile(char* file_name);

	char* combinedPath(char* path_name, char* file_name);
	char* getNewExtensionAndPath( char* old_file_name, const char* extension );

	int lengthBeforeLastOccChar( char* s, char c);
	char* appendCharToString( char* prev_str , char add_c);
	char* substr(char* s, size_t start_ind, size_t length);
	

#endif
