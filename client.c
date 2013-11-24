/*
 * =====================================================================================
 *       Filename:  client.c
 *    Description:  
 *        Created:  2012年12月03日 18时29分02秒
 *       Revision:  none
 *       Compiler:  clang
 *         Author:  szm 
 *         Email :  xianszm007@gmail.com
 *        Company : class 7 of computer science 
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define PORT 9527
int main( int argc, char *argv[] ){
	int sockfd;
	char SEND_BUF[2048]={"FIRST"};
	char RECV_BUF[2048];
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port=htons(PORT);
	bzero(&server.sin_zero,8);
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){
		perror("connect error");
		exit(1);
	}

	while(send(sockfd,SEND_BUF,strlen(SEND_BUF),0)!=0){
		int ret=recv(sockfd,RECV_BUF,2048,0);
		if (ret<0)
		      break;
		else if(ret>0)
		      printf("recv msg from server : \n%s",RECV_BUF);
		memset(SEND_BUF,'\0',2048);
		printf("\nwait for sql_query command >> ");
		fgets(SEND_BUF,2048,stdin);
		if(strncasecmp(SEND_BUF,"quit",4)==0){
			printf("thanks for use !\n Bye !\n");
			break;
		}
	}
	close(sockfd);
	return 0;
}


