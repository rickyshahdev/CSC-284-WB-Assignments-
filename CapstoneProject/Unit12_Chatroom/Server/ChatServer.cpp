#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <sstream>
#include <cstring>
#include <random>
#include <algorithm>

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

// Remove "using namespace std;" and use std:: prefix

// --- Forward Declarations ---
class User;
class Room;
class ChatServer;

// --- User Class ---
class User {
private:
    int socket;
    std::string username;
    Room* currentRoom;
    std::atomic<bool> connected{true};
    
public:
    User(int sock) : socket(sock), currentRoom(nullptr) {}
    
    int getSocket() const { return socket; }
    std::string getUsername() const { return username; }
    Room* getCurrentRoom() const { return currentRoom; }
    bool isConnected() const { return connected; }
    
    void setUsername(const std::string& name) { username = name; }
    void setRoom(Room* room) { currentRoom = room; }
    void disconnect() { connected = false; }
    
    void sendMessage(const std::string& message) {
        if (connected && socket >= 0) {
            send(socket, message.c_str(), message.size(), 0);
        }
    }
};

// --- Room Class ---
class Room {
private:
    std::string name;
    std::vector<User*> users;
    std::mutex usersMutex;
    
public:
    Room(const std::string& roomName) : name(roomName) {}
    
    std::string getName() const { return name; }
    
    void addUser(User* user) {
        std::lock_guard<std::mutex> lock(usersMutex);
        users.push_back(user);
        user->setRoom(this);
        
        // Notify all users in room
        broadcast("[SERVER] " + user->getUsername() + " joined the room.");
    }
    
    void removeUser(User* user) {
        std::lock_guard<std::mutex> lock(usersMutex);
        for (auto it = users.begin(); it != users.end(); ++it) {
            if (*it == user) {
                users.erase(it);
                user->setRoom(nullptr);
                broadcast("[SERVER] " + user->getUsername() + " left the room.");
                break;
            }
        }
    }
    
    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(usersMutex);
        for (User* user : users) {
            user->sendMessage(message + "\n");
        }
    }
    
    void broadcastToOthers(User* sender, const std::string& message) {
        std::lock_guard<std::mutex> lock(usersMutex);
        for (User* user : users) {
            if (user != sender) {
                user->sendMessage(sender->getUsername() + ": " + message + "\n");
            }
        }
    }
    
    std::vector<std::string> getUserList() {
        std::lock_guard<std::mutex> lock(usersMutex);
        std::vector<std::string> userList;
        for (User* user : users) {
            userList.push_back(user->getUsername());
        }
        return userList;
    }
    
    size_t getUserCount() {
        std::lock_guard<std::mutex> lock(usersMutex);
        return users.size();
    }
};

// --- ChatServer Class ---
class ChatServer {
private:
    std::map<std::string, Room*> rooms;
    std::map<int, User*> users;
    std::mutex roomsMutex;
    std::mutex usersMutex;
    std::set<std::string> usedUsernames;
    std::atomic<bool> running{true};
    
    Room* lobby;
    
    std::string generateUniqueUsername(const std::string& baseName) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        
        std::string username = baseName;
        int attempts = 0;
        
        while (usedUsernames.count(username) > 0 && attempts < 100) {
            username = baseName + std::to_string(dis(gen));
            attempts++;
        }
        
        if (usedUsernames.count(username) > 0) {
            username = "user" + std::to_string(dis(gen));
        }
        
        usedUsernames.insert(username);
        return username;
    }
    
public:
    ChatServer() {
        // Create lobby room
        lobby = new Room("Lobby");
        rooms["Lobby"] = lobby;
    }
    
    ~ChatServer() {
        for (auto& pair : rooms) {
            delete pair.second;
        }
        for (auto& pair : users) {
            delete pair.second;
        }
    }
    
    void addUser(int clientSocket) {
        std::lock_guard<std::mutex> lock(usersMutex);
        User* user = new User(clientSocket);
        users[clientSocket] = user;
        
        // Assign default username
        std::string username = generateUniqueUsername("anon");
        user->setUsername(username);
        
        // Add to lobby
        lobby->addUser(user);
        
        user->sendMessage("[SERVER] Welcome to the chat server!\n");
        user->sendMessage("[SERVER] Your username is: " + username + "\n");
        user->sendMessage("[SERVER] You are in the Lobby. Type .HELP for commands.\n");
    }
    
    void removeUser(int clientSocket) {
        std::lock_guard<std::mutex> lock(usersMutex);
        auto it = users.find(clientSocket);
        if (it != users.end()) {
            User* user = it->second;
            
            // Remove from room
            if (user->getCurrentRoom()) {
                user->getCurrentRoom()->removeUser(user);
            }
            
            // Remove username from used list
            usedUsernames.erase(user->getUsername());
            
            delete user;
            users.erase(it);
        }
    }
    
    User* getUser(int clientSocket) {
        std::lock_guard<std::mutex> lock(usersMutex);
        auto it = users.find(clientSocket);
        return (it != users.end()) ? it->second : nullptr;
    }
    
    bool createRoom(const std::string& roomName, User* creator) {
        std::lock_guard<std::mutex> lock(roomsMutex);
        
        if (rooms.count(roomName) > 0) {
            creator->sendMessage("[SERVER] Room '" + roomName + "' already exists.\n");
            return false;
        }
        
        Room* newRoom = new Room(roomName);
        rooms[roomName] = newRoom;
        
        // Move creator to new room
        if (creator->getCurrentRoom()) {
            creator->getCurrentRoom()->removeUser(creator);
        }
        newRoom->addUser(creator);
        
        creator->sendMessage("[SERVER] Created and joined room '" + roomName + "'\n");
        return true;
    }
    
    bool joinRoom(const std::string& roomName, User* user) {
        std::lock_guard<std::mutex> lock(roomsMutex);
        
        auto it = rooms.find(roomName);
        if (it == rooms.end()) {
            user->sendMessage("[SERVER] Room '" + roomName + "' does not exist.\n");
            return false;
        }
        
        Room* room = it->second;
        
        // Leave current room if any
        if (user->getCurrentRoom()) {
            user->getCurrentRoom()->removeUser(user);
        }
        
        // Join new room
        room->addUser(user);
        
        user->sendMessage("[SERVER] Joined room '" + roomName + "'\n");
        return true;
    }
    
    std::vector<std::string> getRoomList() {
        std::lock_guard<std::mutex> lock(roomsMutex);
        std::vector<std::string> roomList;
        for (const auto& pair : rooms) {
            std::string roomInfo = pair.first + " (" + std::to_string(pair.second->getUserCount()) + " users)";
            roomList.push_back(roomInfo);
        }
        return roomList;
    }
    
    void broadcastToRoom(User* sender, const std::string& message) {
        Room* room = sender->getCurrentRoom();
        if (room) {
            if (room->getName() == "Lobby") {
                sender->sendMessage("[SERVER] You cannot chat in the Lobby. Join a room first.\n");
            } else {
                room->broadcastToOthers(sender, message);
                sender->sendMessage("You: " + message + "\n");
            }
        } else {
            sender->sendMessage("[SERVER] You are not in any room. Join a room first.\n");
        }
    }
    
    bool setUsername(User* user, const std::string& requestedName) {
        std::lock_guard<std::mutex> lock(usersMutex);
        
        // Check if username is already taken
        if (usedUsernames.count(requestedName) > 0) {
            return false;
        }
        
        // Remove old username
        usedUsernames.erase(user->getUsername());
        
        // Set new username
        user->setUsername(requestedName);
        usedUsernames.insert(requestedName);
        
        return true;
    }
    
    void handleClientMessage(int clientSocket, const std::string& message) {
        User* user = getUser(clientSocket);
        if (!user || !user->isConnected()) return;
        
        std::cout << "[" << user->getUsername() << "]: " << message << std::endl;
        
        if (message.empty()) return;
        
        // Handle commands starting with '.'
        if (message[0] == '.') {
            std::istringstream iss(message.substr(1));
            std::string command;
            iss >> command;
            
            std::transform(command.begin(), command.end(), command.begin(), ::toupper);
            
            if (command == "CREATE_ROOM") {
                std::string roomName;
                iss >> roomName;
                if (roomName.empty()) {
                    user->sendMessage("[SERVER] Usage: .CREATE_ROOM <room_name>\n");
                } else {
                    createRoom(roomName, user);
                }
            }
            else if (command == "JOIN_ROOM") {
                std::string roomName;
                iss >> roomName;
                if (roomName.empty()) {
                    user->sendMessage("[SERVER] Usage: .JOIN_ROOM <room_name>\n");
                } else {
                    joinRoom(roomName, user);
                }
            }
            else if (command == "LIST_ROOMS") {
                std::vector<std::string> roomList = getRoomList();
                user->sendMessage("[SERVER] Available rooms:\n");
                for (const auto& room : roomList) {
                    user->sendMessage("  " + room + "\n");
                }
            }
            else if (command == "EXIT") {
                user->sendMessage("[SERVER] Goodbye!\n");
                user->disconnect();
            }
            else if (command == "HELP") {
                user->sendMessage("[SERVER] === Available Commands ===\n");
                user->sendMessage("[SERVER] .CREATE_ROOM <name>  - Create a new room\n");
                user->sendMessage("[SERVER] .JOIN_ROOM <name>    - Join an existing room\n");
                user->sendMessage("[SERVER] .LIST_ROOMS          - List all rooms\n");
                user->sendMessage("[SERVER] .EXIT                - Disconnect from server\n");
                user->sendMessage("[SERVER] Any other text is sent as a chat message\n");
            }
            else if (command == "USERNAME") {
                std::string newName;
                iss >> newName;
                if (newName.empty()) {
                    user->sendMessage("[SERVER] Current username: " + user->getUsername() + "\n");
                } else {
                    if (setUsername(user, newName)) {
                        user->sendMessage("[SERVER] Username set to '" + newName + "'\n");
                    } else {
                        std::string autoName = generateUniqueUsername(newName);
                        user->setUsername(autoName);
                        user->sendMessage("[SERVER] Username '" + newName + "' is taken. Your username is now '" + autoName + "'\n");
                    }
                }
            }
            else {
                user->sendMessage("[SERVER] Unknown command: ." + command + "\n");
                user->sendMessage("[SERVER] Type .HELP for available commands\n");
            }
        } else {
            // Regular chat message
            broadcastToRoom(user, message);
        }
    }
};

// --- Global server instance ---
ChatServer* chatServer = nullptr;

void handleClient(int clientSocket) {
    char buffer[4096];
    
    // Add user to server
    chatServer->addUser(clientSocket);
    
    while (true) {
        memset(buffer, 0, 4096);
        int bytes = recv(clientSocket, buffer, 4096, 0);
        
        if (bytes <= 0) break;
        
        std::string message(buffer, bytes);
        // Remove trailing newline if present
        if (!message.empty() && message.back() == '\n') {
            message.pop_back();
        }
        
        chatServer->handleClientMessage(clientSocket, message);
    }
    
    // Remove user from server
    chatServer->removeUser(clientSocket);
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
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ip, &hint.sin_addr);
    
    // Now there's no conflict with std::bind
    int bindResult = bind(serverSocket, (sockaddr*)&hint, sizeof(hint));
    if (bindResult < 0) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        closesocket(serverSocket);
        return 1;
    }
    
    if (listen(serverSocket, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(serverSocket);
        return 1;
    }
    
    std::cout << "Chat Server listening on " << ip << ":" << port << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Create chat server instance
    chatServer = new ChatServer();
    
    // Accept clients
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        
        if (clientSocket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }
        
        // Get client IP
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "New client connected from " << clientIP << std::endl;
        
        std::thread(handleClient, clientSocket).detach();
    }
    
    delete chatServer;
    
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}