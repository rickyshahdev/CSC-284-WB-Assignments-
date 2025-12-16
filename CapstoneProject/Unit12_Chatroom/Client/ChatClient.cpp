#include <ncurses.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>
#include <iostream>
#include <sstream>

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

using namespace std;

// --- Network Manager (for receiving messages in background) ---
class NetworkManager {
private:
    int sock;
    atomic<bool> running{true};
    thread receiveThread;
    vector<string>* messages; // Reference to UI's message list
    mutex* messagesMutex;
    string username;
    
    void receiveMessages() {
        char buffer[4096];
        while (running) {
            memset(buffer, 0, 4096);
            int bytes = recv(sock, buffer, 4096, 0);
            if (bytes > 0) {
                lock_guard<mutex> lock(*messagesMutex);
                messages->push_back(string(buffer, bytes));
            } else if (bytes == 0) {
                lock_guard<mutex> lock(*messagesMutex);
                messages->push_back("Server disconnected.");
                break;
            }
        }
    }
    
public:
    NetworkManager(vector<string>* msg, mutex* mutex) 
        : sock(-1), messages(msg), messagesMutex(mutex) {}  // Fixed initialization order
    
    ~NetworkManager() {
        disconnect();
    }
    
    bool connect(const char* ip = "127.0.0.1", int port = 54000, const string& user = "") {
        username = user;
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;
        
        sockaddr_in serverHint{};
        serverHint.sin_family = AF_INET;
        serverHint.sin_port = htons(port);
        inet_pton(AF_INET, ip, &serverHint.sin_addr);
        
        if (::connect(sock, (sockaddr*)&serverHint, sizeof(serverHint)) < 0) {
            closesocket(sock);
            return false;
        }
        
        // Send username to server
        if (!username.empty()) {
            string userMsg = ".USERNAME " + username;
            send(sock, userMsg.c_str(), userMsg.size(), 0);
        }
        
        // Start background thread for receiving messages
        receiveThread = thread(&NetworkManager::receiveMessages, this);
        return true;
    }
    
    void sendMessage(const string& message) {
        if (sock >= 0) {
            send(sock, message.c_str(), message.size(), 0);
        }
    }
    
    void disconnect() {
        running = false;
        if (sock >= 0) {
            closesocket(sock);
            sock = -1;
        }
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }
};

// --- YOUR ORIGINAL UI COMPONENTS (updated) ---
class UIComponent {
protected:
    WINDOW* window;
    int height, width, startY, startX;

public:
    UIComponent(int h, int w, int y, int x) : height(h), width(w), startY(y), startX(x) {
        window = newwin(h, w, y, x);
    }

    virtual ~UIComponent() {
        if (window) delwin(window);
    }

    virtual void draw() = 0;
    void refreshWin() { wrefresh(window); }
};

class HeaderArea : public UIComponent {
private:
    string currentRoom;
    string username;

public:
    HeaderArea(int w) : UIComponent(3, w, 0, 0), currentRoom("Lobby") {}
    
    void setCurrentRoom(const string& room) {
        currentRoom = room;
    }
    
    void setUsername(const string& user) {
        username = user;
    }
    
    void draw() override {
        werase(window);
        box(window, 0, 0);
        wattron(window, COLOR_PAIR(1) | A_BOLD);
        string title = "C++ Chat Client - User: " + username + " - Room: " + currentRoom;
        mvwprintw(window, 1, (width - title.length()) / 2, "%s", title.c_str());
        wattroff(window, COLOR_PAIR(1) | A_BOLD);
        refreshWin();
    }
};

class RoomList : public UIComponent {
private:
    vector<string> rooms;
    int selectedIndex;

public:
    RoomList(int h, int y) : UIComponent(h, 25, y, 0), selectedIndex(0) {
        // Will be populated by server responses
    }
    
    void setRooms(const vector<string>& newRooms) {
        rooms = newRooms;
        if (selectedIndex >= static_cast<int>(rooms.size())) {
            selectedIndex = max(0, static_cast<int>(rooms.size()) - 1);
        }
    }

    void moveSelection(int direction) {
        selectedIndex += direction;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= static_cast<int>(rooms.size())) selectedIndex = rooms.size() - 1;
    }

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Available Rooms ]");

        for (size_t i = 0; i < rooms.size(); ++i) {
            if (static_cast<int>(i) == selectedIndex) {
                wattron(window, A_REVERSE | COLOR_PAIR(2));
                mvwprintw(window, i + 2, 2, " %-20s ", rooms[i].c_str());
                wattroff(window, A_REVERSE | COLOR_PAIR(2));
            } else {
                mvwprintw(window, i + 2, 2, " %s", rooms[i].c_str());
            }
        }
        refreshWin();
    }
    
    string getSelectedRoom() const {
        if (rooms.empty()) return "";
        return rooms[selectedIndex];
    }
};

class MessageArea : public UIComponent {
private:
    vector<string>* messages;
    mutex* messagesMutex;
    int scrollOffset;

public:
    MessageArea(int h, int w, int y, int x, vector<string>* msg, mutex* mutex) 
        : UIComponent(h, w, y, x), messages(msg), messagesMutex(mutex), scrollOffset(0) {}

    void scrollUp() {
        lock_guard<mutex> lock(*messagesMutex);
        if (scrollOffset < static_cast<int>(messages->size()) - (height - 2)) {
            scrollOffset++;
        }
    }
    
    void scrollDown() {
        if (scrollOffset > 0) {
            scrollOffset--;
        }
    }
    
    void resetScroll() {
        scrollOffset = 0;
    }

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Messages ]");
        
        lock_guard<mutex> lock(*messagesMutex);
        int maxMessages = height - 2;
        int startIdx = max(0, static_cast<int>(messages->size()) - maxMessages - scrollOffset);
        int endIdx = min(static_cast<int>(messages->size()), startIdx + maxMessages);
        
        int line = 1;
        for (int i = startIdx; i < endIdx; ++i) {
            mvwprintw(window, line, 2, "%s", (*messages)[i].c_str());
            line++;
        }
        
        // Show scroll indicator
        if (scrollOffset > 0) {
            mvwprintw(window, 1, width - 3, "^");
        }
        if (static_cast<int>(messages->size()) - scrollOffset > maxMessages) {
            mvwprintw(window, height - 2, width - 3, "v");
        }
        
        refreshWin();
    }
};

class InputArea : public UIComponent {
private:
    string currentInput;
    int cursorPos;

public:
    InputArea(int w, int y) : UIComponent(3, w, y, 0), cursorPos(0) {}

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Compose Message ]");
        mvwprintw(window, 1, 2, "> %s", currentInput.c_str());
        
        // Show cursor
        wmove(window, 1, 3 + cursorPos);
        refreshWin();
    }
    
    void addChar(char ch) {
        if (currentInput.length() < 100) {
            currentInput.insert(cursorPos, 1, ch);
            cursorPos++;
        }
    }
    
    void backspace() {
        if (cursorPos > 0) {
            currentInput.erase(cursorPos - 1, 1);
            cursorPos--;
        }
    }
    
    void deleteChar() {
        if (cursorPos < static_cast<int>(currentInput.length())) {
            currentInput.erase(cursorPos, 1);
        }
    }
    
    void moveCursorLeft() {
        if (cursorPos > 0) cursorPos--;
    }
    
    void moveCursorRight() {
        if (cursorPos < static_cast<int>(currentInput.length())) cursorPos++;
    }
    
    void clear() {
        currentInput.clear();
        cursorPos = 0;
    }
    
    string getInput() {
        string input = currentInput;
        clear();
        return input;
    }
    
    void setInput(const string& text) {
        currentInput = text;
        cursorPos = currentInput.length();
    }
};

class InfoArea : public UIComponent {
private:
    vector<string> infoLines;

public:
    InfoArea(int h, int w, int y, int x) : UIComponent(h, w, y, x) {}
    
    void addInfo(const string& info) {
        infoLines.push_back(info);
        if (infoLines.size() > static_cast<size_t>(height - 2)) {
            infoLines.erase(infoLines.begin());
        }
    }
    
    void clear() {
        infoLines.clear();
    }
    
    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Info ]");
        
        for (size_t i = 0; i < infoLines.size() && i < static_cast<size_t>(height - 2); ++i) {
            mvwprintw(window, i + 1, 2, "%s", infoLines[i].c_str());
        }
        refreshWin();
    }
};

// --- Main Application Controller ---
class ChatClientUI {
private:
    HeaderArea* header;
    RoomList* roomList;
    MessageArea* messageArea;
    InputArea* inputArea;
    InfoArea* infoArea;
    NetworkManager* network;
    vector<string> messages;
    mutex messagesMutex;
    vector<string> serverRooms;
    string currentRoom;
    string username;
    bool isRunning;
    
    void initCurses() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        start_color();
        
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        
        refresh();
    }
    
    void processServerMessage(const string& msg) {
        // Parse server messages and update UI accordingly
        if (msg.find("[SERVER]") != string::npos) {
            if (msg.find("joined room") != string::npos || msg.find("created room") != string::npos) {
                // Extract room name from server message
                size_t pos = msg.find("room '");
                if (pos != string::npos) {
                    size_t end = msg.find("'", pos + 6);
                    if (end != string::npos) {
                        currentRoom = msg.substr(pos + 6, end - pos - 6);
                        header->setCurrentRoom(currentRoom);
                    }
                }
            }
            else if (msg.find("Available rooms:") != string::npos) {
                // Parse room list
                size_t start = msg.find(":");
                if (start != string::npos) {
                    string roomStr = msg.substr(start + 1);
                    istringstream iss(roomStr);
                    string room;
                    serverRooms.clear();
                    while (iss >> room) {
                        if (!room.empty() && room != " ") {
                            serverRooms.push_back(room);
                        }
                    }
                    roomList->setRooms(serverRooms);
                }
            }
            else if (msg.find("Username set to") != string::npos) {
                size_t start = msg.find("to '");
                if (start != string::npos) {
                    size_t end = msg.find("'", start + 4);
                    if (end != string::npos) {
                        username = msg.substr(start + 4, end - start - 4);
                        header->setUsername(username);
                    }
                }
            }
        }
    }
    
    void sendCommand(const string& command) {
        if (command.empty()) return;
        
        if (command == ".EXIT") {
            isRunning = false;
            return;
        }
        
        network->sendMessage(command);
        
        if (command == ".LIST_ROOMS") {
            lock_guard<mutex> lock(messagesMutex);
            messages.push_back("You requested room list...");
        }
    }

public:
    ChatClientUI(int argc, char* argv[]) : currentRoom("Lobby"), isRunning(true) {  // Fixed initialization order
#ifdef _WIN32
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
#endif

        initCurses();
        
        // Parse command line arguments
        const char* ip = "127.0.0.1";
        int port = 54000;
        string user = "";
        
        if (argc >= 2) ip = argv[1];
        if (argc >= 3) port = atoi(argv[2]);
        if (argc >= 4) user = argv[3];
        
        // Initialize network
        network = new NetworkManager(&messages, &messagesMutex);
        
        if (!network->connect(ip, port, user)) {
            endwin();
            cout << "Failed to connect to server " << ip << ":" << port << endl;
            exit(1);
        }
        
        // Initialize UI
        int maxH, maxW;
        getmaxyx(stdscr, maxH, maxW);
        
        int headerH = 3;
        int inputH = 3;
        int infoH = 8;
        int mainContentH = maxH - headerH - inputH - infoH;
        
        header = new HeaderArea(maxW);
        if (user.empty()) {
            header->setUsername("Connecting...");
        } else {
            header->setUsername(user);
        }
        header->setCurrentRoom(currentRoom);
        
        roomList = new RoomList(mainContentH, headerH);
        messageArea = new MessageArea(mainContentH, maxW - 25, headerH, 25, &messages, &messagesMutex);
        inputArea = new InputArea(maxW, maxH - inputH - infoH);
        infoArea = new InfoArea(infoH, maxW, maxH - infoH, 0);
        
        // Add welcome message
        messages.push_back("Connected to chat server.");
        messages.push_back("Type .HELP for available commands.");
        messages.push_back("Current room: Lobby");
        
        infoArea->addInfo("=== Chat Commands ===");
        infoArea->addInfo(".CREATE_ROOM <name> - Create new room");
        infoArea->addInfo(".JOIN_ROOM <name>   - Join existing room");
        infoArea->addInfo(".LIST_ROOMS         - List all rooms");
        infoArea->addInfo(".EXIT               - Disconnect and quit");
        infoArea->addInfo("Other text is sent as chat message");
    }

    ~ChatClientUI() {
        delete header;
        delete roomList;
        delete messageArea;
        delete inputArea;
        delete infoArea;
        delete network;
        endwin();
        
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void run() {
        // Removed unused variable 'inCommandMode'
        
        while (isRunning) {
            // Process incoming messages from server
            {
                lock_guard<mutex> lock(messagesMutex);
                if (!messages.empty()) {
                    // Process the latest message from server
                    string latestMsg = messages.back();
                    processServerMessage(latestMsg);
                }
            }
            
            // Draw all components
            header->draw();
            roomList->draw();
            messageArea->draw();
            inputArea->draw();
            infoArea->draw();
            
            // Handle Input
            int ch = getch();
            
            if (ch == KEY_F(1)) { // F1 for help
                infoArea->addInfo("=== Quick Help ===");
                infoArea->addInfo("Use commands starting with '.'");
                infoArea->addInfo("Arrow keys: Navigate room list");
                infoArea->addInfo("PgUp/PgDn: Scroll messages");
                infoArea->addInfo("F2: Request room list");
                infoArea->addInfo("ESC: Clear input");
                infoArea->addInfo("F10: Exit");
                continue;
            }
            
            if (ch == KEY_F(2)) { // F2 to list rooms
                sendCommand(".LIST_ROOMS");
                continue;
            }
            
            if (ch == KEY_F(10)) { // F10 to exit
                sendCommand(".EXIT");
                continue;
            }
            
            if (ch == KEY_PPAGE) {
                messageArea->scrollUp();
                continue;
            }
            
            if (ch == KEY_NPAGE) {
                messageArea->scrollDown();
                continue;
            }
            
            if (ch == 27) { // ESC key
                inputArea->clear();
                continue;
            }
            
            if (ch == KEY_BACKSPACE || ch == 127) {
                inputArea->backspace();
                continue;
            }
            
            if (ch == KEY_DC) { // Delete key
                inputArea->deleteChar();
                continue;
            }
            
            if (ch == KEY_LEFT) {
                inputArea->moveCursorLeft();
                continue;
            }
            
            if (ch == KEY_RIGHT) {
                inputArea->moveCursorRight();
                continue;
            }
            
            if (ch == KEY_UP) {
                roomList->moveSelection(-1);
                continue;
            }
            
            if (ch == KEY_DOWN) {
                roomList->moveSelection(1);
                continue;
            }
            
            if (ch == '\n' || ch == KEY_ENTER) {
                string input = inputArea->getInput();
                
                if (input.empty()) continue;
                
                if (input[0] == '.') {
                    // Process command
                    sendCommand(input);
                } else {
                    // Send chat message
                    if (currentRoom == "Lobby") {
                        lock_guard<mutex> lock(messagesMutex);
                        messages.push_back("[SERVER] You must join a room first!");
                    } else {
                        network->sendMessage(input);
                        lock_guard<mutex> lock(messagesMutex);
                        messages.push_back("You: " + input);
                    }
                }
                continue;
            }
            
            // Regular character input
            if (ch >= 32 && ch <= 126) {
                inputArea->addChar(static_cast<char>(ch));
            }
        }
    }
};

// --- Entry Point ---
int main(int argc, char* argv[]) {
    ChatClientUI app(argc, argv);
    app.run();
    return 0;
}