/*
 * =====================================================================================
 *
 *       Filename:  init_socket.cpp
 *        Version:  1.0
 *        Created:  2013年11月21日 15时32分11秒
 *       Revision:  none
 *       Compiler:  clang
 *         Author:  sim szm, xianszm007@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "init_socket.h"
#define PORT 9527
#define backlog 10
int init_socket(){
	int listenfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in address;
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bzero(&address,sizeof(struct sockaddr_in));
	address.sin_family=AF_INET;
	address.sin_port=htons(PORT);
	address.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(listenfd,(struct sockaddr*)&address,sizeof(address))==-1){
		exit(0);
	}
	if( listen(listenfd, backlog) == -1) {
        	exit(0);
    	}
	return listenfd;
}
