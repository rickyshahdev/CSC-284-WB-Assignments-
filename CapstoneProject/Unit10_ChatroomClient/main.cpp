// Standard headers and ncurses for terminal UI
#include <ncurses.h>
#include <vector>
#include <string>
#include <iostream>

// --- Abstract Base / Helper Class ---
// UIComponent is a small wrapper around an ncurses WINDOW with
// height/width/position stored so derived classes can implement
// custom draw behavior. It also manages the window lifetime.
class UIComponent {
protected:
    WINDOW* window;
    int height, width, startY, startX;

public:
// Constructor
    UIComponent(int h, int w, int y, int x) : height(h), width(w), startY(y), startX(x) {
        window = newwin(h, w, y, x);
    }

    // Virtual destructor ensures derived destructors run and
    // we free the ncurses WINDOW when this component is destroyed.
    virtual ~UIComponent() {
        if (window) delwin(window);
    }

    // draw() must be implemented by derived classes to display
    // the component contents into the WINDOW.
    virtual void draw() = 0;

    // Helper to refresh the underlying ncurses window.
    void refreshWin() {
        wrefresh(window);
    }
};

// --- Component: Header ---
// Small header bar at the top of the screen that displays a title.
class HeaderArea : public UIComponent {
public:
    // Fixed height of 3 rows for the header
    HeaderArea(int w) : UIComponent(3, w, 0, 0) {}

    // Draw the header: border + title using a colored attribute
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
// Left-side component that lists available rooms and manages a
// selected index. Supports keyboard movement to change selection.
class RoomList : public UIComponent {
private:
    std::vector<std::string> rooms; // list of room names
    int selectedIndex;              // currently highlighted index

public:
    RoomList(int h, int y) : UIComponent(h, 20, y, 0), selectedIndex(0) {
        // Sample/static rooms for demo purposes. In a real client
        // these would be loaded from the server or a config.
        rooms = {"General", "Sports", "Programming", "Off-Topic", "Announcements"};
    }

    // Move the selection up/down; keeps index inside bounds.
    void moveSelection(int direction) {
        selectedIndex += direction;
        if (selectedIndex < 0) selectedIndex = 0;
        if (selectedIndex >= static_cast<int>(rooms.size())) selectedIndex = rooms.size() - 1;
    }

    // Render the room list, highlighting the selected entry.
    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Rooms ]");

        for (size_t i = 0; i < rooms.size(); ++i) {
            if (static_cast<int>(i) == selectedIndex) {
                wattron(window, A_REVERSE | COLOR_PAIR(2)); // Highlight selected
                mvwprintw(window, i + 2, 2, " %-14s ", rooms[i].c_str());
                wattroff(window, A_REVERSE | COLOR_PAIR(2));
            } else {
                // Non-selected entries show a simple prefix marker
                mvwprintw(window, i + 2, 2, " #%s", rooms[i].c_str());
            }
        }
        refreshWin();
    }
    
    // Return the currently selected room name.
    std::string getSelectedRoom() const {
        return rooms[selectedIndex];
    }
};

// --- Component: Message Area ---
// Main message display area (center-right). Shows a list of recent
// messages. Currently uses a simple vector of strings as placeholder
// data; in a real client this would be fed by the network layer.
class MessageArea : public UIComponent {
private:
    std::vector<std::string> messages; // stored messages to display

public:
    MessageArea(int h, int w, int y, int x) : UIComponent(h, w, y, x) {
        // Placeholder messages for demo purposes
        messages.push_back("System: Welcome to the chat!");
        messages.push_back("User1: Hello everyone.");
        messages.push_back("User2: Working on the Unit 10 assignment?");
    }

    // Draw messages top-to-bottom, clipping to the window height.
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
// Bottom input area for composing messages (currently static placeholder)
class InputArea : public UIComponent {
public:
    InputArea(int w, int y) : UIComponent(3, w, y, 0) {}

    // Draw a small compose box; actual input handling is not implemented
    // in this demo and would require echoing and line-editing logic.
    void draw() override {
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 0, 2, "[ Compose ]");
        mvwprintw(window, 1, 2, "> Type a message here... (Press Q to quit)");
        refreshWin();
    }
};

// --- Main Application Controller ---
// Top-level UI controller that wires the components together and
// drives the main input/render loop.
class ChatClientUI {
private:
    HeaderArea* header;
    RoomList* roomList;
    MessageArea* messageArea;
    InputArea* inputArea;
    bool isRunning;

    // Initialize ncurses settings and color pairs used by the UI.
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
    // Construct the UI, establish layout using the terminal size,
    // and create the component objects.
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

    // Clean up and end curses mode. Deleting components will free
    // their associated ncurses WINDOW objects.
    ~ChatClientUI() {
        delete header;
        delete roomList;
        delete messageArea;
        delete inputArea;
        endwin(); // End curses mode
    }

    // Main loop: draw UI and handle a small set of inputs for demo.
    void run() {
        while (isRunning) {
            // Draw all components
            header->draw();
            roomList->draw();
            messageArea->draw();
            inputArea->draw();

            // Handle Input: arrow keys change room selection; Q quits.
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