#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <sstream>

class Book {
public:
    std::string title;
    std::string author;
    int year;
    std::string genre;

    Book() = default;
    Book(const std::string& t, const std::string& a, int y, const std::string& g)
        : title(t), author(a), year(y), genre(g) {}

    // Method to return formatted book details: "Title by Author (Year) [Genre]"
    std::string display() const {
        std::ostringstream oss;
        oss << title << " by " << author << " (" << year << ") [" << genre << "]";
        return oss.str();
    }

    // For sorting convenience
    bool operator<(const Book& other) const {
        return year < other.year;
    }
};

#endif // BOOK_H
