all: server client
debug:
	g++ -pthread server.cpp ourudpftp.cpp -o server -g
	g++ -pthread client.cpp ourudpftp.cpp -o client -g

server: server.cpp ourudpftp.cpp
	g++ -pthread server.cpp ourudpftp.cpp -o server

client: client.cpp ourudpftp.cpp
	g++ -pthread client.cpp ourudpftp.cpp -o client

clean:
	rm server client
