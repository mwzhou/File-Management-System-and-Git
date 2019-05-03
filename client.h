#ifndef CLIENT_H
#define CLIENT_H

#include "fileHelperMethods.h"


//MACROS///////////////////////////////////////////////////////////////////////
#define recieveStringSocket(sockfd) recieveStringSocketst( sockfd, "Server")
#define sendStringSocket(sockfd, str) sendStringSocketst(sockfd, str, "Server")

#define recieveFileSocket( sockfd, dir_to_store ) recieveFileSocketst( sockfd, dir_to_store, "Server" )
#define sendFileSocket( sockfd, file_name )  sendFileSocketst(sockfd, file_name, "Server")

#define sendTarFile( sockfd, file_path, dir_to_store)	sendTarFilest( sockfd, file_path, dir_to_store,  "Server" )
#define recieveTarFile( sockfd, dir_to_store) recieveTarFilest( sockfd, dir_to_store , "Server")
/////////////////////////////////////////////////////////////////////////


void sendArgsToServer(char* s1, char* s2, char* s3);


#endif
