/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *        Version:  1.0
 *        Created:  2013年11月20日 19时03分02秒
 *       Revision:  none
 *       Compiler:  clang
 *         Author:  sim szm, xianszm007@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#include <unistd.h>
#include <cassert>
#include <iostream>
#include "process_pool.h"
#include "work.h"
#include "init_socket.h"
#define process_num 8
int main(int argc,char *argv[]){
	int listenfd=init_socket();
	assert(listenfd!=-1);
	process_pool<work>* pool =process_pool<work>::init_pool(listenfd,process_num);
	if(pool){
		pool->run_pool();
		delete pool;
	}
	close(listenfd);
	return 0;
}
