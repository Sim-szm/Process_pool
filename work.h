/*
 * =====================================================================================
 *
 *       Filename:  work.h
 *        Version:  1.0
 *        Created:  2013年11月18日 17时25分35秒
 *       Revision:  none
 *       Compiler:  clang
 *         Author:  sim szm, xianszm007@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */
#ifndef WORK_H
#define WORK_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define buf_size 256 
#define BUFF 22
class work{
public:
	void init(int sockfd,struct sockaddr_in client_address);
	int process();
private:
	char user_buffer[buf_size];
	char msg[BUFF];
	int connfd;
	struct sockaddr_in address;
};

#endif
