Compile
Server

g++ ChatServer.cpp -o chatserver -std=c++17 -pthread

Runserver

./chatserver

CLient

g++ ChatClient.cpp -o chatclient -lncurses -pthread -std=c++17

Run client

./chatclient 127.0.0.1 54000