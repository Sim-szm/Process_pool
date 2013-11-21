/*
 * =====================================================================================
 *
 *       Filename:  work.cpp
 *        Version:  1.0
 *        Created:  2013年11月21日 15时44分39秒
 *       Revision:  none
 *       Compiler:  clang
 *         Author:  sim szm, xianszm007@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <aio.h>
#include "work.h"
void work::init(int sockfd,struct sockaddr_in client_address){
	connfd=sockfd;
	address=client_address;
}
int work::process(){
	int ret=recv(connfd,user_buffer,buf_size,0);
	if(ret<0)
	      return -1;
	else if(ret>0){
		std::cout<<"client "<<(char*)inet_ntoa(address.sin_addr)<<" send data :  "<<user_buffer<<std::endl;
		memset(user_buffer,'\0',buf_size);
		memset(msg,'\0',BUFF);
		int fd=open("msg.txt",O_RDONLY|O_NONBLOCK);
		read(fd,msg,BUFF);
		close(fd);
		send(connfd,msg,BUFF,0);
	}
	return ret;
}
