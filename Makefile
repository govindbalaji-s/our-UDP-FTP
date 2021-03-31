all: server client

server: server.cpp ourudpftp.cpp
	g++ server.cpp ourudpftp.cpp -o server

client: client.cpp ourudpftp.cpp
	g++ client.cpp ourudpftp.cpp -o client

clean:
	rm server client
