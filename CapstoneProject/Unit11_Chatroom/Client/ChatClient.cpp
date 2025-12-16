// Your original UI with networking added
#include <ncurses.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>
#include <iostream>

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

// --- Network Manager (for receiving messages in background) ---
class NetworkManager {
private:
    int sock;
    std::atomic<bool> running{true};
    std::thread receiveThread;
    std::vector<std::string>* messages; // Reference to UI's message list
    std::mutex* messagesMutex;
    
    void receiveMessages() {
        char buffer[4096];
        while (running) {
            memset(buffer, 0, 4096);
            int bytes = recv(sock, buffer, 4096, 0);
            if (bytes > 0) {
                std::lock_guard<std::mutex> lock(*messagesMutex);
                messages->push_back(std::string(buffer, bytes));
            } else if (bytes == 0) {
                break; // Server disconnected
            }
        }
    }
    
public:
    NetworkManager(std::vector<std::string>* msg, std::mutex* mutex) 
        : messages(msg), messagesMutex(mutex), sock(-1) {}
    
    ~NetworkManager() {
        disconnect();
    }
    
    bool connect(const char* ip = "127.0.0.1", int port = 54000) {
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
        
        // Start background thread for receiving messages
        receiveThread = std::thread(&NetworkManager::receiveMessages, this);
        return true;
    }
    
    void sendMessage(const std::string& message) {
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
public:
    HeaderArea(int w) : UIComponent(3, w, 0, 0) {}
    void draw() override {
        werase(window);
        box(window, 0, 0);
        wattron(window, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(window, 1, 2, "ADVANCED C++ CHAT CLIENT - Unit 10 Capstone");
        wattroff(window, COLOR_PAIR(1) | A_BOLD);
        refreshWin();
    }
};

class RoomList : public UIComponent {
private:
    std::vector<std::string> rooms;
    int selectedIndex;

public:
    RoomList(int h, int y) : UIComponent(h, 20, y, 0), selectedIndex(0) {
        rooms = {"General", "Sports", "Programming", "Off-Topic", "Announcements"};
    }

    void moveSelection(int direction) {
        selectedIndex += direction;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= static_cast<int>(rooms.size())) selectedIndex = rooms.size() - 1;
    }

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Rooms ]");

        for (size_t i = 0; i < rooms.size(); ++i) {
            if (static_cast<int>(i) == selectedIndex) {
                wattron(window, A_REVERSE | COLOR_PAIR(2));
                mvwprintw(window, i + 2, 2, " %-14s ", rooms[i].c_str());
                wattroff(window, A_REVERSE | COLOR_PAIR(2));
            } else {
                mvwprintw(window, i + 2, 2, " #%s", rooms[i].c_str());
            }
        }
        refreshWin();
    }
    
    std::string getSelectedRoom() const {
        return rooms[selectedIndex];
    }
};

class MessageArea : public UIComponent {
private:
    std::vector<std::string>* messages;
    std::mutex* messagesMutex;

public:
    MessageArea(int h, int w, int y, int x, std::vector<std::string>* msg, std::mutex* mutex) 
        : UIComponent(h, w, y, x), messages(msg), messagesMutex(mutex) {}

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Messages ]");

        std::lock_guard<std::mutex> lock(*messagesMutex);
        int line = 1;
        for (const auto& msg : *messages) {
            if (line < height - 1) {
                mvwprintw(window, line, 2, "%s", msg.c_str());
                line++;
            }
        }
        refreshWin();
    }
};

class InputArea : public UIComponent {
public:
    InputArea(int w, int y) : UIComponent(3, w, y, 0) {}

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Compose ]");
        mvwprintw(window, 1, 2, "> ");
        refreshWin();
    }
    
    // Get user input (simple version)
    std::string getInput() {
        echo();
        char input[256];
        mvwgetnstr(window, 1, 4, input, 255);
        noecho();
        return std::string(input);
    }
};

// --- Main Application Controller ---
class ChatClientUI {
private:
    HeaderArea* header;
    RoomList* roomList;
    MessageArea* messageArea;
    InputArea* inputArea;
    NetworkManager* network;
    std::vector<std::string> messages;
    std::mutex messagesMutex;
    bool isRunning;

    void initCurses() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        start_color();
        
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        
        refresh();
    }

public:
    ChatClientUI(int argc, char* argv[]) : isRunning(true) {
#ifdef _WIN32
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
#endif

        initCurses();
        
        // Parse command line
        const char* ip = "127.0.0.1";
        int port = 54000;
        if (argc >= 2) ip = argv[1];
        if (argc >= 3) port = atoi(argv[2]);
        
        // Initialize network
        network = new NetworkManager(&messages, &messagesMutex);
        
        if (!network->connect(ip, port)) {
            endwin();
            std::cout << "Failed to connect to server " << ip << ":" << port << std::endl;
            exit(1);
        }
        
        // Initialize UI
        int maxH, maxW;
        getmaxyx(stdscr, maxH, maxW);
        
        int headerH = 3;
        int inputH = 3;
        int mainContentH = maxH - headerH - inputH;
        
        header = new HeaderArea(maxW);
        roomList = new RoomList(mainContentH, headerH);
        messageArea = new MessageArea(mainContentH, maxW - 20, headerH, 20, &messages, &messagesMutex);
        inputArea = new InputArea(maxW, maxH - inputH);
        
        // Add welcome message
        messages.push_back("Connected to chat server. Type messages below.");
        messages.push_back("Use arrow keys to change rooms, Q to quit.");
    }

    ~ChatClientUI() {
        delete header;
        delete roomList;
        delete messageArea;
        delete inputArea;
        delete network;
        endwin();
        
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void run() {
        while (isRunning) {
            // Draw all components
            header->draw();
            roomList->draw();
            messageArea->draw();
            inputArea->draw();
            
            // Handle Input
            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    roomList->moveSelection(-1);
                    break;
                case KEY_DOWN:
                    roomList->moveSelection(1);
                    break;
                case '\n':
                case KEY_ENTER: {
                    // Get message from user
                    std::string input = inputArea->getInput();
                    
                    if (input.empty()) break;
                    
                    if (input == "/quit" || input == "/exit") {
                        isRunning = false;
                        break;
                    }
                    
                    // Send to server
                    network->sendMessage(input);
                    
                    // Show in our UI
                    {
                        std::lock_guard<std::mutex> lock(messagesMutex);
                        messages.push_back("You: " + input);
                    }
                    break;
                }
                case 'q':
                case 'Q':
                    isRunning = false;
                    break;
                default:
                    break;
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
