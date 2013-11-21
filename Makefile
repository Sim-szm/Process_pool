all: init_socket.o work.o server

init_socket.o: init_socket.cpp init_socket.h
	g++ -c init_socket.cpp -o init_socket.o
work.o: work.cpp work.h
	g++ -c work.cpp -o work.o
server: main.cpp process_pool.h init_socket.o work.o
	g++ process_pool.h init_socket.o work.o main.cpp -o server
clean:
	rm *.o server stress_test
.client: client
client:
	g++ stress_test.cpp -o stress_test
