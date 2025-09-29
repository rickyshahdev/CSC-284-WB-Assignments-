#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <utility>

// Provide make_unique for compilers that don't support C++14
#if __cplusplus < 201402L
namespace std {
    template <class T, class... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

using namespace std;

class Document {
  public:
    string title;
    string content;

    Document(const string &title, const string &content) : title(title), content(content) {

    }

    ~Document() {
        // Cleanup resources if any
        cout << "Document '" << title << "' is being destroyed." << endl;
    }

    void show() const {
        cout << "Title: " << title << "\nContent: " << content << endl;
    }

    void edit(const string &newTitle, const string &newContent) {
        title = newTitle;
        content = newContent;
    }

    unique_ptr<Document> createDocument(const string &title, const string &content) {
        return unique_ptr<Document>(new Document(title, content));
    }

    shared_ptr<Document> shareDocument(const string &title, const string &content) {
        return shared_ptr<Document>(new Document(title, content));
    }

};

int main() {
    // simple demo using Document and the smart-pointer factories
    auto d = make_unique<Document>("Sample", "This is a sample document.");
    // containers to hold created documents so they live until we explicitly destroy them
    // use raw pointers for storedDocs to work with older default compiler modes; we delete them on exit
    vector<Document*> storedDocs;
    vector<shared_ptr<Document> > storedSharedDocs;

    

    while (true) {
        cout << "Document Management System Menu:\n"
             << "1. Create Document (unique_ptr)\n"
             << "2. Create Document (shared_ptr)\n"
             << "3. Show current Document\n"
             << "4. Edit current Document\n"
             << "5. Exit\n"
             << "Enter your choice: ";
        char choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case '1': {
                string title, content;
                cout << "Enter document title: ";
                getline(cin, title);
                cout << "Enter document content: ";
                getline(cin, content);
                auto docPtr = d->createDocument(title, content);
                // release ownership into the raw-pointer container (we will delete on option 5)
                storedDocs.push_back(docPtr.release());
                cout << "Created and stored document (unique_ptr)." << endl;
                break;
            }
            case '2': {
                string title, content;
                cout << "Enter document title: ";
                getline(cin, title);
                cout << "Enter document content: ";
                getline(cin, content);
                auto docShared = d->shareDocument(title, content);
                storedSharedDocs.push_back(docShared);
                cout << "Created and stored document (shared_ptr)." << endl;
                break;
            }
            case '3':
                if (d) d->show(); else cout << "No current document." << endl;
                // also show stored documents summary
                if (!storedDocs.empty() || !storedSharedDocs.empty()) {
                    cout << "\nStored documents:\n";
                    for (size_t i = 0; i < storedDocs.size(); ++i) {
                        cout << "=== Unique_ptr demonstration ===" << endl;
                        if (storedDocs[i]) storedDocs[i]->show();
                    }
                    for (size_t i = 0; i < storedSharedDocs.size(); ++i) {
                        cout << "=== Shared_ptr demonstration ==="<< endl;
                        storedSharedDocs[i]->show();
                    }
                }
                break;
            case '4': {
                if (!d) {
                    cout << "No current document to edit." << endl;
                    break;
                }
                cout << "Edit selection (0: current, 1: stored unique, 2: stored shared): ";
                char editChoice;
                cin >> editChoice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (editChoice == '0') {
                    string newTitle, newContent;
                    cout << "Enter new title: ";
                    getline(cin, newTitle);
                    cout << "Enter new content: ";
                    getline(cin, newContent);
                    d->edit(newTitle, newContent);
                    cout << "Current document updated successfully." << endl;
                } else if (editChoice == '1') {
                    if (storedDocs.empty()) {
                        cout << "No stored unique documents to edit." << endl;
                    } else {
                        cout << "Stored unique documents:\n";
                        for (size_t i = 0; i < storedDocs.size(); ++i) {
                            cout << i << ": " << (storedDocs[i] ? storedDocs[i]->title : string("<null>")) << '\n';
                        }
                        cout << "Select index of document to edit: ";
                        string idxLine;
                        getline(cin, idxLine);
                        if (idxLine.empty()) getline(cin, idxLine);
                        try {
                            size_t idx = static_cast<size_t>(stoul(idxLine));
                            if (idx >= storedDocs.size() || !storedDocs[idx]) {
                                cout << "Invalid index." << endl;
                            } else {
                                cout << "Enter new title: ";
                                string selNewTitle;
                                getline(cin, selNewTitle);
                                cout << "Enter new content: ";
                                string selNewContent;
                                getline(cin, selNewContent);
                                storedDocs[idx]->edit(selNewTitle, selNewContent);
                                cout << "Stored unique document updated." << endl;
                            }
                        } catch (...) {
                            cout << "Invalid input." << endl;
                        }
                    }
                } else if (editChoice == '2') {
                    if (storedSharedDocs.empty()) {
                        cout << "No stored shared documents to edit." << endl;
                    } else {
                        cout << "Stored shared documents:\n";
                        for (size_t i = 0; i < storedSharedDocs.size(); ++i) {
                            cout << i << ": " << (storedSharedDocs[i] ? storedSharedDocs[i]->title : string("<null>")) << '\n';
                        }
                        cout << "Select index of shared document to edit: ";
                        string idxLine;
                        getline(cin, idxLine);
                        if (idxLine.empty()) getline(cin, idxLine);
                        try {
                            size_t idx = static_cast<size_t>(stoul(idxLine));
                            if (idx >= storedSharedDocs.size() || !storedSharedDocs[idx]) {
                                cout << "Invalid index." << endl;
                            } else {
                                cout << "Enter new title: ";
                                string selNewTitle;
                                getline(cin, selNewTitle);
                                cout << "Enter new content: ";
                                string selNewContent;
                                getline(cin, selNewContent);
                                storedSharedDocs[idx]->edit(selNewTitle, selNewContent);
                                cout << "Stored shared document updated." << endl;
                            }
                        } catch (...) {
                            cout << "Invalid input." << endl;
                        }
                    }
                } else {
                    cout << "Invalid selection." << endl;
                }
                break;
            }
            case '5':
                // explicitly destroy stored documents and the current Document
                for (auto p : storedDocs) delete p;
                storedDocs.clear();
                storedSharedDocs.clear();
                d.reset();
                cout << "All stored documents and the current document destroyed. Exiting Document Management System." << endl;
                return 0;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
}