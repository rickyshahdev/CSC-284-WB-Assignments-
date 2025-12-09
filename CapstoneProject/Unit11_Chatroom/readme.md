To Compile:
g++ -std=c++11 ChatServer.cpp -o server -pthread

g++ -std=c++11 ChatClient.cpp -o client -pthread -lncurses

Start server

./server

Connect to specific IP with default port
./server 127.0.0.1

Connect to specific IP and port
./server 127.0.0.1 54000

Start Ui client

./client

Connect to specific IP with default port
./client 127.0.0.1

Connect to specific IP and port
./client 127.0.0.1 54000
