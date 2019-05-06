#ifndef FILE_HELP
#define FILE_HELP
#include "structures.h"

//ERROR MACROS
	#define PRINT_ERROR(txt) ({\
				if(errno!=0)\
					fprintf(stderr, "\n\tERROR:%s\n\terrno:%s\n\tFile:%s  on Line:%d  in Func:%s()\n\n", txt, strerror(errno), __FILE__, __LINE__, __func__);\
				else\
					fprintf(stderr, "\n\tERROR:%s\n\tFile:%s  on Line:%d  in Func:%s()\n\n", txt, __FILE__, __LINE__, __func__);\
			})


	#define pRETURN_ERROR(txt,ret_val) do{  PRINT_ERROR(txt); return ret_val; }while(0)
	#define pRETURN_ERRORvoid(txt) do{  PRINT_ERROR(txt); return; }while(0)

	#define pEXIT_ERROR(txt) do{ PRINT_ERROR(txt); exit(EXIT_FAILURE); }while(0)


//MACROS FOR READABILITY
	#define WRITE_AND_CHECKe(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pEXIT_ERROR("write()"); } }while(0) //writes to file, if failed, prints out error and returns void
	#define WRITE_AND_CHECKn(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pRETURN_ERROR("write()", NULL); } }while(0) //writes to file, if failed, prints out error and returns null
	#define WRITE_AND_CHECKb(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pRETURN_ERROR("write()", false); } }while(0) //writes to file, if failed, prints out error and returns void
	#define WRITE_AND_CHECKv(file, buf, nbytes) do{  if( write(file, buf , nbytes) < 0 ) { pRETURN_ERRORvoid("write()"); } }while(0) //writes to file, if failed, prints out error and returns void


	#define READ_AND_CHECKe(file, buf, nbytes) do{ if( read(file, buf , nbytes)<0 ) pEXIT_ERROR("read()"); }while(0)
	#define READ_AND_CHECKn(file, buf, nbytes) do{ if( read(file, buf , nbytes)<0 ) pRETURN_ERROR("read()", NULL); }while(0)


	#define REMOVE_AND_CHECK(file_name) do{ if(remove(file_name) == 0) fprintf( stderr, "\n\tremoved file:%s\n",file_name); else fprintf( stderr, "couldn't remove file:%s",file_name);  }while(0) //removes file and prints if successful
	#define TESTP printf("\ntest: %d\n", __LINE__)


	#define sendErrorSocket(sockfd) sendNumSocket(sockfd, -1)
	#define SUCCESS_SEND 1766


//ENUMS
	//enum to differentiate between FileTypes
	typedef enum{ isDIR=017, isREG=736, isUNDEF=-1 }FileType;


//METHOD SIGNATURES

	/*File Manipulation Methods*/

	int extractLine(char* fpath, char* target);
	int sizeOfFile(char* file_name);
	char* readFile(char* file_name);
	int openFileW(char* file_name);
	FileType typeOfFile(char* file_name);
	bool moveFile( char* file_path , char* dir_to_store);
	bool removeDir( char* dir );
	bool copyFile(char* file_name, char* copy_path);
	bool fileEquals(char* f1_name, char* f2_path);
	int numFilesInDir( char* dir_name );
	char* findFileMatchInDir( char* dir_name, char* f_compare );

	/*String Manipulation Methods*/
	char* substr(char* s, size_t start_ind, size_t length);
	char* combinedPath(char* path_name, char* file_name);
	char* concatString(char* s1, char* s2);
	char* copyString( char* s1 );

	int lengthBeforeLastOccChar( char* s, char c);

	/*Socket Methods*/
	bool sendSig( int sockfd, bool err_cmp);
	bool receiveSig( int sockfd );

	bool sendNumSocket( int sockfd, int num );

	bool sendStringSocketst( int sockfd, char* str, char* sock_type );
	char* recieveStringSocketst( int sockfd, char* sock_type );

	bool sendFileSocketst( int sockfd, char* file_name, char* sock_type );
	char* recieveFileSocketst( int sockfd, char* dir_to_store , char* sock_type );

	/*Tar Methods*/
	bool sendTarFilest( int sockfd, char* file_path, char* dir_to_store, char* sock_type );
	char* recieveTarFilest( int sockfd, char* dir_to_store , char* sock_type);
	char* unTar( char* tar_filepath );
	char* makeTar(char* proj_name, char* path_File);

	/*Manifest Methods*/
	char* createManifest(char* proj_name);
	bool writeToManifest(char* path, int  manifest_fd );
	char* generateHash (char* file_name);

	/*WTF methods*/
	int getProjectVersion( char* proj_name );

#endif
