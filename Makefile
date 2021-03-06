all: init_socket.o work.o server

init_socket.o: init_socket.cpp init_socket.h
	g++ -c init_socket.cpp -o init_socket.o
work.o: work.cpp work.h
	g++ -c work.cpp -o work.o -I /usr/include/mysql/
server: main.cpp process_pool.h init_socket.o work.o
	g++ process_pool.h init_socket.o work.o main.cpp -o server -lmysqlclient -lm -g
clean:
	rm *.o server stress_test sql_work
.client: client
client:
	g++ stress_test.cpp -o stress_test
.work: work
work:
	gcc client.c -o sql_work
