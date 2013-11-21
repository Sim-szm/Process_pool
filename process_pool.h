/*
 * =====================================================================================
 *
 *       Filename:  process_poll.h
 *        Version:  1.0
 *        Created:  2013年11月18日 13时52分04秒
 *       Revision:  none
 *       Compiler:  clang
 *         Author:  sim szm, xianszm007@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */
#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
class process{
public:
	process():pid_(-1){}
public :
	pid_t pid_;
	int pipefd[2];
};
template<typename T>
class process_pool{
public:
	static process_pool<T>* init_pool(int listenfd,int process_num=8){
		if(!instance_){
			instance_=new process_pool<T> (listenfd,process_num);
		}
		return instance_;
	}
	~process_pool(){
		delete [] sub_process_;
	}
	void run_pool();
private:
	process_pool(int listenfd,int process_num=8 );
private:
	void run_parent_process();
	void run_child_process();
	void set_signal();
private:
	static const int max_process_num=16;
	static const int user_num_per_process=65533;
	static const int max_event_num=10000;
	int epollfd_;
	int listenfd_;
	int stop_;
	int process_num_;
	int index_;
	process* sub_process_;
public:
	static process_pool<T> * instance_;
};
static int signal_pipefd[2];
template<typename T>
process_pool<T>* process_pool<T>::instance_=NULL;

static int setnonblocking(int fd){
	if(fcntl(fd,F_SETFL,fcntl(fd,F_GETFD,0)|O_NONBLOCK)==-1){
		return -1;
	}
	return 0;
}
static void set_fd(int epollfd_,int fd){
	struct epoll_event ev;
	setnonblocking(fd);
	ev.data.fd=fd;
	ev.events=EPOLLIN|EPOLLET;
	epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev);
}
static inline void del_fd(int epollfd_,int fd){
	epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,0);
	close(fd);
}
static inline void set_sig(int sig,void(handler)(int),bool restart=true){
	struct sigaction sa;
	bzero(&sa,sizeof(sa));
	sa.sa_handler=handler;
	if(restart)
	      sa.sa_flags|=SA_RESTART;
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig,&sa,NULL)!=-1);
}
void signal_handler(int sig){
	int msg=sig;
	send(signal_pipefd[1],(char*)&msg,1,0);
}

template<typename T>
process_pool<T>::process_pool(int listenfd,int process_num):listenfd_(listenfd),process_num_(process_num),index_(-1),stop_(false){
	sub_process_=new process[process_num];
	assert(sub_process_);
	for(int i=0;i<process_num;i++){
		int pair_fd=socketpair(PF_UNIX,SOCK_STREAM,0,sub_process_[i].pipefd);
		assert(pair_fd==0);
		sub_process_[i].pid_=fork();
		assert(sub_process_[i].pid_>=0);
		if(sub_process_[i].pid_>0){
			close(sub_process_[i].pipefd[1]);
			continue;
		}else{
			close(sub_process_[i].pipefd[0]);
			index_=i;
			break;
		}
	}
}

template<typename T>
void process_pool<T>::set_signal(){
	epollfd_=epoll_create(10);
	assert(epollfd_!=-1);
	int pair_fd=socketpair(PF_UNIX,SOCK_STREAM,0,signal_pipefd);
	assert(pair_fd==0);
	setnonblocking(signal_pipefd[1]);
	set_fd(epollfd_,signal_pipefd[0]);
	set_sig(SIGINT,signal_handler);
	set_sig(SIGCHLD,signal_handler);
	set_sig(SIGTERM,signal_handler);
}
template<typename T>
void process_pool<T>::run_pool(){
	if(index_!=-1){
		run_child_process();
		return ;
	}
	run_parent_process();
}
template<typename T>
void process_pool<T>::run_child_process(){
	set_signal();
	int pipefd=sub_process_[index_].pipefd[1];
	set_fd(epollfd_,pipefd);
	struct epoll_event events[max_event_num];
	T* users=new T[user_num_per_process];
	assert(users);
	int current_num=0;
	int ret=-1;
	while(!stop_){
		current_num=epoll_wait(epollfd_,events,max_event_num,-1);
		if(current_num<0 && errno!=EINTR)
		      break;
		for(int i=0;i<current_num;i++){
			int sockfd=events[i].data.fd;
			if((sockfd==pipefd)&&(events[i].events & EPOLLIN)){
				int client=0;
				ret=recv(sockfd,(char*)&client,sizeof(client),0);
				if((ret<0 && (errno!=EAGAIN))||ret==0){
					continue;
				}else{
					struct sockaddr_in address;
					socklen_t len=sizeof(address);
					int connfd=accept(listenfd_,(struct sockaddr*)&address,&len);
					if(connfd<0)
					      continue;
					set_fd(epollfd_,connfd);
					users[connfd].init(connfd,address);
				}
			}else if((sockfd==signal_pipefd[0])&&(events[i].events&EPOLLIN)){
				int sig;
				char signals[1024];
				ret=recv(signal_pipefd[0],signals,sizeof(signals),0);
				if(ret<0)
				      continue;
				else{
					for(int i=0;i<ret;i++){
						switch(signals[i]){
							case SIGCHLD:
								pid_t pid;
								int stat;
								while((pid=waitpid(-1,&stat,WNOHANG))>0)
								      continue;
								break;
							case SIGINT:
								stop_=true;
								break;
							default:
								break;
						}
					}
				}
			}else if(events[i].events&EPOLLIN){
				if((users[sockfd].process())<0)
				      del_fd(epollfd_,sockfd);
				else 
				      continue;
			}
			else
			      continue;
		}
	}
	delete []users;
	users=NULL;
	close(pipefd);
	close(epollfd_);
}
template<typename T>
void process_pool<T>::run_parent_process(){
	set_signal();
	set_fd(epollfd_,listenfd_);
	struct epoll_event events[max_event_num];
	int sub_process_counter=0;
	int new_conn=1;
	int current_num=0;
	int ret=-1;
	while(!stop_){
		current_num=epoll_wait(epollfd_,events,max_event_num,-1);
		if(current_num<0 &&errno!=EINTR){
			break;
		}
		for(int i=0;i<current_num;i++){
			int sockfd=events[i].data.fd;
			if(sockfd==listenfd_){
				int i=sub_process_counter;
				do{
					if(sub_process_[i].pid_!=-1)
					      break;
					i=(i+1)%process_num_;
				}while(i!=sub_process_counter);
				if(sub_process_[i].pid_==-1){
					stop_=true;
					break;
				}
				sub_process_counter=(i+1)%process_num_;
				send(sub_process_[i].pipefd[0],(char*)&new_conn,sizeof(new_conn),0);
				std::cout<<"send request to child_process "<<i<<std::endl;
			}else if((sockfd==signal_pipefd[0])&&(events[i].events&EPOLLIN)){
				int sig;
				char signals[1024];
				ret=recv(signal_pipefd[0],signals,sizeof(signals),0);
				if(ret<=0)
				      continue;
				else{
					for(int i=0;i<ret;i++){
						switch(signals[i]){
							case SIGCHLD:
								pid_t pid;
								int stat;
								while((pid=waitpid(-1,&stat,WNOHANG))>0){
									for(int i=0;i<process_num_;i++){
										if(sub_process_[i].pid_==pid){
											close(sub_process_[i].pipefd[0]);
											sub_process_[i].pid_=-1;
										}
									}
								}
								stop_=true;
								for(int i=0;i<process_num_;i++){
									if(sub_process_[i].pid_!=-1)
									      stop_=false;
								}
								break;
							case SIGINT:
								std::cout<<"kill all child now !"<<std::endl;
								for(int i=0;i<process_num_;i++){
									pid_t pid=sub_process_[i].pid_;
									if(pid!=-1){
										kill(pid,SIGTERM);
									}
								}
								break;
							default:
								break;
						}
					}
				}
			}else
			      continue;
		}
	}
	close(epollfd_);
	//close(listenfd_);
}

#endif
