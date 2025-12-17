
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <sstream> // For generating unique IDs
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
#define INVALID_SOCKET -1
#endif

using namespace std;

// Forward declarations for mutual dependencies
class Room;
class User;

// --- User Class (Replaces struct Client) ---
class User {
public:
    SOCKET sock;
    string username;
    Room* currentRoom = nullptr; // Pointer to the room the user is in

    User(SOCKET s, const string& name) : sock(s), username(name) {}

    void send(const string& message) const {
        ::send(sock, message.c_str(), (int)message.size(), 0);
    }
};

// --- Room Class ---
class Room {
private:
    string name;
    vector<User*> members;
    mutex roomMutex;

public:
    Room(const string& roomName) : name(roomName) {}

    const string& getName() const { return name; }
    size_t getMemberCount() const { return members.size(); }

    void addUser(User* user) {
        lock_guard<mutex> lock(roomMutex);
        members.push_back(user);
    }

    void removeUser(User* user) {
        lock_guard<mutex> lock(roomMutex);
        members.erase(
            remove(members.begin(), members.end(), user),
            members.end()
        );
    }

    void broadcast(const string& message, User* sender = nullptr) {
        lock_guard<mutex> lock(roomMutex);
        for (User* c : members) {
            // Optional: Skip sending the message back to the sender if needed
            // if (c != sender) {
                c->send(message);
            // }
        }
    }
};


// --- ChatServer Class (Encapsulates all server state) ---
class ChatServer {
private:
    unordered_map<string, Room*> rooms;
    unordered_map<SOCKET, User*> clients; // Map socket to User object
    unordered_map<string, SOCKET> usernames; // Map username to socket (for uniqueness check)
    mutex globalMutex;
    SOCKET listeningSocket;

    // Helper to generate a unique default username
    string getUniqueDefaultUsername() {
        static int anonCount = 1000;
        string name;
        lock_guard<mutex> lock(globalMutex);
        do {
            name = "anon" + to_string(anonCount++);
        } while (usernames.count(name));
        return name;
    }

    // Handles an incoming message from a client
    void handleClient(User* user) {
        char buffer[4096];

        // Place new user in the Lobby (which is auto-created)
        joinRoom(user, "Lobby");
        user->send("[SERVER] Welcome! Your username is: " + user->username + "\n");
        user->send("[SERVER] You are in the 'Lobby'. Use .LIST_ROOMS to see rooms.\n");

        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(user->sock, buffer, sizeof(buffer), 0);
            
            if (bytesReceived <= 0) {
                // Client disconnected
                cout << user->username << " disconnected." << endl;
                cleanUpUser(user);
                break;
            }

            string msg(buffer, bytesReceived);
            // Commands
            if (msg.rfind('.', 0) == 0) {
                processCommand(user, msg);
            } else {
                // Regular chat message
                if (user->currentRoom) {
                    string fullMsg = user->username + ": " + msg;
                    user->currentRoom->broadcast(fullMsg);
                    cout << "Broadcasted to " << user->currentRoom->getName() << ": " << fullMsg << endl;
                } else {
                    user->send("[SERVER] You must join a room first!\n");
                }
            }
        }
    }

    // Handles all client-side commands
    void processCommand(User* user, const string& msg) {
        // .USERNAME command is handled client-side at startup in the new client design, 
        // but we'll keep the logic here for robustness, e.g., for name changes.
        if (msg.rfind(".USERNAME ", 0) == 0) {
            string newName = msg.substr(10);
            updateUsername(user, newName);

        } else if (msg.rfind(".CREATE_ROOM ", 0) == 0) {
            string roomName = msg.substr(13);
            if (rooms.count(roomName)) {
                user->send("[SERVER] Room '" + roomName + "' already exists.\n");
            } else {
                createRoom(roomName);
                user->send("[SERVER] Room '" + roomName + "' created.\n");
                // Auto-join the newly created room
                joinRoom(user, roomName); 
            }

        } else if (msg.rfind(".JOIN_ROOM ", 0) == 0) {
            string roomName = msg.substr(11);
            joinRoom(user, roomName);

        } else if (msg == ".LIST_ROOMS") {
            string reply = "[SERVER] Available rooms: ";
            lock_guard<mutex> lock(globalMutex);
            for (auto const& [name, room] : rooms) {
                reply += name + " (" + to_string(room->getMemberCount()) + ") ";
            }
            user->send(reply + "\n");

        } else if (msg == ".EXIT") {
            cout << user->username << " requested disconnect." << endl;
            closesocket(user->sock);
            cleanUpUser(user);
        } else {
            user->send("[SERVER] Unknown command: " + msg + "\n");
        }
    }

    // Handles user joining a room, including leaving the old one
    void joinRoom(User* user, const string& roomName) {
        lock_guard<mutex> lock(globalMutex);
        if (user->currentRoom && user->currentRoom->getName() == roomName) {
            user->send("[SERVER] You are already in room '" + roomName + "'.\n");
            return;
        }

        if (rooms.count(roomName)) {
            // 1. Remove from old room
            if (user->currentRoom) {
                user->currentRoom->broadcast("[SERVER] " + user->username + " left the room.\n");
                user->currentRoom->removeUser(user);
            }

            // 2. Add to new room
            Room* newRoom = rooms[roomName];
            newRoom->addUser(user);
            user->currentRoom = newRoom;
            
            // 3. Notify
            user->send("[SERVER] You successfully joined room '" + roomName + "'.\n");
            newRoom->broadcast("[SERVER] " + user->username + " joined the room.\n", user);
        } else {
            user->send("[SERVER] Room '" + roomName + "' does not exist.\n");
        }
    }
    
    // Creates a room and adds it to the map
    void createRoom(const string& roomName) {
        lock_guard<mutex> lock(globalMutex);
        if (!rooms.count(roomName)) {
            rooms[roomName] = new Room(roomName);
            cout << "Room created: " << roomName << endl;
        }
    }

    // Handles username uniqueness check and update
    void updateUsername(User* user, string& newName) {
        // Sanitize input (optional, but good practice)
        if (newName.empty() || newName.find(' ') != string::npos || newName.length() > 20) {
            user->send("[SERVER] Invalid username. Must be non-empty, no spaces, max 20 chars.\n");
            return;
        }

        lock_guard<mutex> lock(globalMutex);
        if (usernames.count(newName) && usernames[newName] != user->sock) {
            // Username is taken by another connected user
            string reply = "[SERVER] Username '" + newName + "' is taken. Please choose another.\n";
            user->send(reply);
            return;
        }

        // Update username maps and User object
        if (!user->username.empty()) {
            usernames.erase(user->username);
        }
        
        user->username = newName;
        usernames[newName] = user->sock;

        string reply = "[SERVER] Username set to '" + user->username + "'\n";
        user->send(reply);
        cout << user->sock << " set username to: " << user->username << endl;
    }

    // Handles cleanup when a client disconnects or exits
    void cleanUpUser(User* user) {
        lock_guard<mutex> lock(globalMutex);
        
        // 1. Remove from room
        if (user->currentRoom) {
            user->currentRoom->broadcast("[SERVER] " + user->username + " disconnected.\n");
            user->currentRoom->removeUser(user);
        }

        // 2. Remove from global client/username maps
        clients.erase(user->sock);
        usernames.erase(user->username);

        // 3. Clean up resources
        closesocket(user->sock);
        delete user;
    }

public:
    ChatServer(int port) : listeningSocket(INVALID_SOCKET) {
        // Initialize WinSock on Windows
        #ifdef _WIN32
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
        #endif

        // Create the initial "Lobby" room
        createRoom("Lobby");

        // Setup the listening socket
        listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listeningSocket == INVALID_SOCKET) {
            cerr << "Can't create socket!" << endl;
            // Handle error/exit
            return;
        }

        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);
        inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

        if (::bind(listeningSocket, (sockaddr*)&hint, sizeof(hint)) < 0) {
            cerr << "Failed to bind to port " << port << endl;
            closesocket(listeningSocket);
            // Handle error/exit
            return;
        }

        if (listen(listeningSocket, SOMAXCONN) < 0) {
            cerr << "Listen failed" << endl;
            closesocket(listeningSocket);
            // Handle error/exit
            return;
        }

        cout << "Server started on port " << port << ". Waiting for connections..." << endl;
    }

    ~ChatServer() {
        // Clean up memory for rooms and clients
        for (auto const& [key, room] : rooms) {
            delete room;
        }
        for (auto const& [key, user] : clients) {
            delete user;
        }

        closesocket(listeningSocket);
        #ifdef _WIN32
        WSACleanup();
        #endif
    }

    void run() {
        while (true) {
            sockaddr_in clientHint;
            socklen_t clientSize = sizeof(clientHint);
            SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientHint, &clientSize);
            
            if (clientSocket < 0) continue;

            // Generate a unique default username
            string defaultName = getUniqueDefaultUsername();

            // Create new User object (replaces old Client struct)
            User* newUser = new User(clientSocket, defaultName);
            
            // Register user globally
            {
                lock_guard<mutex> lock(globalMutex);
                clients[clientSocket] = newUser;
                usernames[defaultName] = clientSocket;
            }

            cout << "New connection accepted. Assigned username: " << defaultName << endl;
            
            // Handle client in a new thread
            thread t(&ChatServer::handleClient, this, newUser);
            t.detach();
        }
    }
};

// --- Main Function (Simplified) ---
int main(int argc, char* argv[]) {
    int port = 54000;
    if (argc >= 2) {
        port = atoi(argv[1]);
    }

    ChatServer server(port);
    server.run();

    return 0;
}