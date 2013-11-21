#include <iostream>
#include <cassert>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define MAX_EVENT_NUM 10000
#define BUFF_SIZE 2048
static const char* request = "GET http://localhost/index.html HTTP/1.1\r\nConnection: keep-alive\r\n\r\n";

int setnonblocking( int fd ) {
    if(fcntl( fd, F_SETFL,fcntl(fd,F_GETFD,0)|O_NONBLOCK)==-1)
        return -1;
    return 0;
}

void set_fd( int epoll_fd, int fd ) {
    epoll_event ev;
    setnonblocking( fd );
    ev.data.fd = fd;
    ev.events = EPOLLOUT | EPOLLET | EPOLLERR;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
}

bool write_nbytes( int sockfd, const char* buffer, int len ) {
    int bytes_write = 0;
    std::cout<<"write out" <<len<<"bytes to socket :"<<sockfd<<std::endl;;
    while( 1 ) {
        bytes_write = send( sockfd, buffer, len, 0 );
        if ( bytes_write == -1 ) {
            return false;
        } else if ( bytes_write == 0 ) {
            return false;
        }

        len -= bytes_write;
        buffer = buffer + bytes_write;
        if ( len <= 0 ) {
            return true;
        }
    }
}

bool read_once( int sockfd, char* buffer, int len ) {
    int bytes_read = 0;
    memset( buffer, '\0', len );
    bytes_read = recv( sockfd, buffer, len, 0 );
    if ( bytes_read == -1 ) {
        return false;
    } else if ( bytes_read == 0 ) {
        return false;
    }
    std::cout<<"read in "<<bytes_read<< " bytes from socket"<<sockfd<<" with content:" <<buffer<<std::endl;

    return true;
}

void start_conn( int epoll_fd, int num, const char* ip, int port ) {
    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    for ( int i = 0; i < num; ++i ) {
        int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
	sleep(1);
	std::cout<<"create "<<i<<" sock"<<std::endl;
        if( sockfd < 0 ) {
            continue;
        }
        if (  connect( sockfd, ( struct sockaddr* )&address, sizeof( address ) ) == 0  ) {
		std::cout<<"build connection "<<i<<std::endl;
            	set_fd( epoll_fd, sockfd );
        }
    }
}

void close_conn( int epoll_fd, int sockfd ) {
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, sockfd, 0 );
    close( sockfd );
}

int main( int argc, char* argv[] ) {
    if(argc!=4){
	  std::cout<<"Usage : "<<argv[0]<< " < IP > < PORT > <connect number>"<<std::endl;
	  _exit(1);
    }
    int epoll_fd = epoll_create( 10 );
    start_conn( epoll_fd, atoi( argv[ 3 ] ), argv[1], atoi( argv[2] ) );
    epoll_event events[ MAX_EVENT_NUM ];
    char buffer[ BUFF_SIZE ];
    while ( true ) {
        int fds = epoll_wait( epoll_fd, events, MAX_EVENT_NUM, 2000 );
        for ( int i = 0; i < fds; i++ ) {
            int sockfd = events[i].data.fd;
            if ( events[i].events & EPOLLIN ) {
                if ( ! read_once( sockfd, buffer, BUFF_SIZE ) ) {
                    close_conn( epoll_fd, sockfd );
                }
                struct epoll_event event;
                event.events = EPOLLOUT | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl( epoll_fd, EPOLL_CTL_MOD, sockfd, &event );
            } else if( events[i].events & EPOLLOUT ) {
                if ( ! write_nbytes( sockfd, request, strlen( request ) ) ) {
                    close_conn( epoll_fd, sockfd );
                }
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl( epoll_fd, EPOLL_CTL_MOD, sockfd, &event );
            } else if( events[i].events & EPOLLERR ) {
                close_conn( epoll_fd, sockfd );
            }
        }
    }
    return 0;
}

