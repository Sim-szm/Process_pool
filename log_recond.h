#ifndef _LOG_RECOND_H
#define _LOG_RECOND_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_MESSAGE 256
#define ERROR_MODE_TYPE 1
#define SYSTEM_MODE_TYPE 0
char LOG_PATH[]="./";
typedef struct {
	int fd;
	pthread_mutex_t log_locker;
}Log_t;
typedef struct {
	Log_t error_log;
	Log_t system_log; 
}LOG_TYPE;
int Create_Log(int log_type,char *filename,LOG_TYPE *log_recond){
	int fd;
	char fileinfo[100];
	mkdir("./log",S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	strcpy(fileinfo,LOG_PATH);
	strcat(fileinfo,filename);
	printf("%s\n",fileinfo);
	fd=open(fileinfo,O_RDWR|O_CREAT,0666);
	printf("%x",O_RDWR|O_CREAT);
	if(fd<0){
		fprintf(stderr,"Error while create log file!\n");
		return -1;
	}
	if(log_type==1) {
	  log_recond->error_log.fd=fd;
	  pthread_mutex_init(&log_recond->error_log.log_locker,NULL);	
	  fprintf(stderr,"error_Logfile created\n");
	  close(log_recond->error_log.fd);
       	}
	if(log_type==0){
		log_recond->system_log.fd=fd;
		pthread_mutex_init(&log_recond->system_log.log_locker,NULL);
		fprintf(stderr,"system_logfile created\n");
		close(log_recond->system_log.fd);
	}
	return 0;
}
int Write_Log(int log_type,char *message,LOG_TYPE *log_recond){
	int writenlen;
	time_t currnet_time;
	char message_info[MAX_MESSAGE];
	int fd;
	time(&currnet_time);
	strcpy(message_info,ctime(&currnet_time));
	message_info[strlen(ctime(&currnet_time))-1]=' ';
	strcat(message_info,message);
	strcat(message_info,"\n");
	if(log_type==1){
		fd=open("error_log.txt",O_RDWR|O_CREAT|O_APPEND);
		log_recond->error_log.fd=fd;
	    	pthread_mutex_lock(&log_recond->error_log.log_locker);
		writenlen=write(log_recond->error_log.fd,message_info,strlen(message_info));
		pthread_mutex_unlock(&log_recond->error_log.log_locker);
		close(log_recond->error_log.fd);
		if(writenlen<0)
		  return -1;
	}
	if(log_type==0){
		fd=open("system_log.txt",O_RDWR|O_CREAT|O_APPEND);
		log_recond->system_log.fd=fd;
		pthread_mutex_lock(&log_recond->system_log.log_locker);
		writenlen=write(log_recond->system_log.fd,message_info,strlen(message_info));
		pthread_mutex_unlock(&log_recond->system_log.log_locker);
		close(log_recond->system_log.fd);
		if(writenlen<0)
		  return -1;
	}
	return 0;
}
#endif
