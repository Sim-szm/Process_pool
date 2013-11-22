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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <mysql.h>
#include "work.h"
void work::init(int sockfd,struct sockaddr_in client_address){
	connfd=sockfd;
	address=client_address;
}
void work::Interface_For_Mysql(char *search_line){
	MYSQL *conn;
    	MYSQL_RES *res;
	MYSQL_ROW row;
  	const char *server = "localhost";
    	const char *user = "root";
    	const char *password = "xianszm007";
    	const char *database = "sim";
    	conn = mysql_init(NULL);
    	if (!mysql_real_connect(conn,server,user,password,database,0, NULL, 0)) {
		std::cout<< mysql_error(conn)<<std::endl;
       		return;
    	}
    	if ( mysql_set_character_set(conn, "utf8" ) ) {
		std::cout<< mysql_error(conn)<<std::endl;
		return;
    	}
	if(search_line!=NULL){
    		if(!mysql_query(conn, search_line)){}
		else
		      return;
    	}else
	      return;
	res = mysql_use_result(conn);
    	while ((row = mysql_fetch_row(res))!=NULL){
		for(int t=0;t<mysql_num_fields(res);t++){
			strcat(msg,row[t]);
			strcat(msg," | ");
		}
		strcat(msg,"\n");
	}
    	mysql_free_result(res);
    	mysql_close(conn);
	send(connfd,msg,BUFF,0);
}
int work::process(){
	memset(user_buffer,'\0',buf_size);
	int ret=recv(connfd,user_buffer,buf_size,0);
	if(ret<0)
	      return -1;
	else if(strncasecmp(user_buffer,"FIRST",5)!=0){
		std::cout<<"client "<<(char*)inet_ntoa(address.sin_addr)<<" send data :  "<<user_buffer<<std::endl;
		memset(msg,'\0',BUFF);
		Interface_For_Mysql(user_buffer);
	}else{
		int fd=open("msg.txt",O_RDONLY|O_NONBLOCK);
		int ret=read(fd,user_buffer,buf_size);
		close(fd);
		send(connfd,user_buffer,buf_size,0);
	}
	return ret;
}

