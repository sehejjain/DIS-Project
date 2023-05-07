p2p: server client
server: server.cpp setNetwork.cpp 
	g++ server.cpp setNetwork.cpp -std=c++11 -o server -g
client: client.cpp fileutils.cpp setNetwork.cpp userId.cpp
	g++ client.cpp setNetwork.cpp fileutils.cpp userId.cpp -std=c++11 -pthread -lm -o client -g
clean:
	rm -f server client *.o test.jpg output/*
	rm -r client.dSYM server.dSYM
	make