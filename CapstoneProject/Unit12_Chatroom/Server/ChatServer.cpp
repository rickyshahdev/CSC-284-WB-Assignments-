
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <unordered_map>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
typedef int SOCKET;
#endif

using namespace std;

mutex coutMutex;
mutex clientsMutex;

struct Client {
    SOCKET sock;
    string username;
    string currentRoom;
};

vector<Client> clients;
unordered_map<string, vector<Client*>> rooms;

void broadcastToRoom(const string& room, const string& message) {
    lock_guard<mutex> lock(clientsMutex);
    if (rooms.find(room) != rooms.end()) {
        for (Client* c : rooms[room]) {
            send(c->sock, message.c_str(), (int)message.size(), 0);
        }
    }
}

void handleClient(Client* client) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(client->sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            lock_guard<mutex> lock(clientsMutex);
            coutMutex.lock();
            cout << client->username << " disconnected." << endl;
            coutMutex.unlock();
            // Remove from room
            if (!client->currentRoom.empty() && rooms.count(client->currentRoom)) {
                auto& v = rooms[client->currentRoom];
                v.erase(remove(v.begin(), v.end(), client), v.end());
            }
            // Remove from clients
            clients.erase(remove_if(clients.begin(), clients.end(),
                                    [client](const Client& c){ return c.sock==client->sock; }),
                          clients.end());
            closesocket(client->sock);
            delete client;
            break;
        }

        string msg(buffer, bytesReceived);

        lock_guard<mutex> lock(coutMutex);
        cout << "[Command received] From " << client->username << ": " << msg << endl;

        // Commands
        if (msg.rfind(".USERNAME ", 0) == 0) {
            client->username = msg.substr(10);
            cout << "-> Set username to: " << client->username << endl;
            string reply = "[SERVER] Username set to '" + client->username + "'";
            send(client->sock, reply.c_str(), (int)reply.size(), 0);
        }
        else if (msg.rfind(".CREATE_ROOM ", 0) == 0) {
            string roomName = msg.substr(13);
            rooms[roomName]; // create empty room if not exists
            cout << "-> Room created: " << roomName << endl;
            string reply = "[SERVER] Room '" + roomName + "' created.";
            send(client->sock, reply.c_str(), (int)reply.size(), 0);
        }
        else if (msg.rfind(".JOIN_ROOM ", 0) == 0) {
            string roomName = msg.substr(11);
            if (rooms.count(roomName)) {
                if (!client->currentRoom.empty()) {
                    auto& v = rooms[client->currentRoom];
                    v.erase(remove(v.begin(), v.end(), client), v.end());
                }
                client->currentRoom = roomName;
                rooms[roomName].push_back(client);
                cout << "-> " << client->username << " joined room: " << roomName << endl;
                string reply = "[SERVER] You joined room '" + roomName + "'";
                send(client->sock, reply.c_str(), (int)reply.size(), 0);
            } else {
                string reply = "[SERVER] Room '" + roomName + "' does not exist.";
                send(client->sock, reply.c_str(), (int)reply.size(), 0);
            }
        }
        else if (msg == ".LIST_ROOMS") {
            string reply = "[SERVER] Available rooms: ";
            for (auto& pair : rooms) reply += pair.first + " ";
            cout << "-> Listed rooms" << endl;
            send(client->sock, reply.c_str(), (int)reply.size(), 0);
        }
        else if (msg == ".EXIT") {
            cout << "-> " << client->username << " requested disconnect." << endl;
            closesocket(client->sock);
            break;
        }
        else {
            // regular chat message
            if (!client->currentRoom.empty()) {
                string fullMsg = client->username + ": " + msg;
                broadcastToRoom(client->currentRoom, fullMsg);
                cout << "-> Broadcasted message to room " << client->currentRoom << ": " << msg << endl;
            } else {
                string reply = "[SERVER] You must join a room first!";
                send(client->sock, reply.c_str(), (int)reply.size(), 0);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    const char* ip = "0.0.0.0";
    int port = 54000;
    if (argc >= 2) port = atoi(argv[1]);

#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2,2), &data);
#endif

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ip, &hint.sin_addr);

    // Use proper type for sizeof on macOS: socklen_t
    if (::bind(listening, (sockaddr*)&hint, sizeof(hint)) < 0) {
        cerr << "Failed to bind" << endl;
        return 1;
    }

    if (listen(listening, SOMAXCONN) < 0) {
        cerr << "Listen failed" << endl;
        return 1;
    }

    cout << "Server started on port " << port << ". Waiting for connections..." << endl;

    while (true) {
        sockaddr_in clientHint;
        socklen_t clientSize = sizeof(clientHint);
        SOCKET clientSocket = accept(listening, (sockaddr*)&clientHint, &clientSize);
        if (clientSocket < 0) continue;

        auto* client = new Client{clientSocket, "Anonymous", ""};
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(*client);
        }

        cout << "New connection accepted." << endl;

        thread t(handleClient, client);
        t.detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
