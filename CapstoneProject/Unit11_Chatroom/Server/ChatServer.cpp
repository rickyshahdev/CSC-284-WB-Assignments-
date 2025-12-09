#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#endif

std::vector<int> clients;
std::mutex clientsMutex;

void handleClient(int clientSocket) {
    char buffer[4096];
    
    while (true) {
        memset(buffer, 0, 4096);
        int bytes = recv(clientSocket, buffer, 4096, 0);
        
        if (bytes <= 0) break;
        
        // Broadcast to other clients
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int client : clients) {
            if (client != clientSocket) {
                send(client, buffer, bytes, 0);
            }
        }
    }
    
    // Remove client
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i] == clientSocket) {
                clients.erase(clients.begin() + i);
                break;
            }
        }
    }
    closesocket(clientSocket);
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
#endif

    const char* ip = "127.0.0.1";
    int port = 54000;
    
    if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = atoi(argv[2]);
    
    // Create server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ip, &hint.sin_addr);
    
    bind(serverSocket, (sockaddr*)&hint, sizeof(hint));
    listen(serverSocket, SOMAXCONN);
    
    std::cout << "Server listening on port " << port << std::endl;
    
    // Accept clients
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }
        
        std::thread(handleClient, clientSocket).detach();
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}