#include <ncurses.h>
#include <vector>
#include <string>
#include <iostream>

// --- Abstract Base / Helper Class ---
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

    virtual void draw() = 0; // Pure virtual function

    void refreshWin() {
        wrefresh(window);
    }
};

// --- Component: Header ---
class HeaderArea : public UIComponent {
public:
    HeaderArea(int w) : UIComponent(3, w, 0, 0) {}

    void draw() override {
        werase(window);
        box(window, 0, 0);
        wattron(window, COLOR_PAIR(1) | A_BOLD); // Use Color Pair 1
        mvwprintw(window, 1, 2, "ADVANCED C++ CHAT CLIENT - Unit 10 Capstone");
        wattroff(window, COLOR_PAIR(1) | A_BOLD);
        refreshWin();
    }
};

// --- Component: Room List ---
class RoomList : public UIComponent {
private:
    std::vector<std::string> rooms;
    int selectedIndex;

public:
    RoomList(int h, int y) : UIComponent(h, 20, y, 0), selectedIndex(0) {
        // Sample Rooms
        rooms = {"General", "Sports", "Programming", "Off-Topic", "Announcements"};
    }

    void moveSelection(int direction) {
        selectedIndex += direction;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= rooms.size()) selectedIndex = rooms.size() - 1;
    }

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Rooms ]");

        for (size_t i = 0; i < rooms.size(); ++i) {
            if (i == selectedIndex) {
                wattron(window, A_REVERSE | COLOR_PAIR(2)); // Highlight selected
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

// --- Component: Message Area ---
class MessageArea : public UIComponent {
private:
    std::vector<std::string> messages;

public:
    MessageArea(int h, int w, int y, int x) : UIComponent(h, w, y, x) {
        // Placeholder messages
        messages.push_back("System: Welcome to the chat!");
        messages.push_back("User1: Hello everyone.");
        messages.push_back("User2: Working on the Unit 10 assignment?");
    }

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Messages ]");

        int line = 1;
        for (const auto& msg : messages) {
            if (line < height - 1) {
                mvwprintw(window, line, 2, "%s", msg.c_str());
                line++;
            }
        }
        refreshWin();
    }
};

// --- Component: Input Area ---
class InputArea : public UIComponent {
public:
    InputArea(int w, int y) : UIComponent(3, w, y, 0) {}

    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Compose ]");
        mvwprintw(window, 1, 2, "> Type a message here... (Press Q to quit)");
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
    bool isRunning;

    void initCurses() {
        initscr();            // Start curses mode
        cbreak();             // Line buffering disabled
        noecho();             // Don't echo while we do getch
        keypad(stdscr, TRUE); // Enable F-keys and arrows
        start_color();        // Enable colors

        // Define Color Pairs (Foreground, Background)
        init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Header text
        init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Selected room
        
        refresh(); // Refresh main stdscr
    }

public:
    ChatClientUI() : isRunning(true) {
        initCurses();

        int maxH, maxW;
        getmaxyx(stdscr, maxH, maxW);

        // Layout Calculations
        int headerH = 3;
        int inputH = 3;
        int mainContentH = maxH - headerH - inputH;

        // Initialize Components
        header = new HeaderArea(maxW);
        roomList = new RoomList(mainContentH, headerH);
        messageArea = new MessageArea(mainContentH, maxW - 20, headerH, 20);
        inputArea = new InputArea(maxW, maxH - inputH);
    }

    ~ChatClientUI() {
        delete header;
        delete roomList;
        delete messageArea;
        delete inputArea;
        endwin(); // End curses mode
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
int main() {
    ChatClientUI app;
    app.run();
    return 0;
}