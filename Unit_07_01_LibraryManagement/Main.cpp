#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "Book.h"
#include "Library.h"

// Functor for filtering by author (replaces lambda)
struct AuthorFilter {
    std::string author;
    AuthorFilter(const std::string& a) : author(a) {}
    bool operator()(const Book& b) const {
        return b.author == author;
    }
};

// Functor for filtering by year
struct YearFilter {
    int year;
    YearFilter(int y) : year(y) {}
    bool operator()(const Book& b) const {
        return b.year > year;
    }
};

// Functor for filtering by genre
struct GenreFilter {
    std::string genre;
    GenreFilter(const std::string& g) : genre(g) {}
    bool operator()(const Book& b) const {
        return b.genre == genre;
    }
};

// Functor for sorting by year ascending
struct SortByYearAsc {
    bool operator()(const Book& a, const Book& b) const {
        return a.year < b.year;
    }
};

// Functor for sorting by year descending
struct SortByYearDesc {
    bool operator()(const Book& a, const Book& b) const {
        return a.year > b.year;
    }
};

int main() {
    // Create library
    Library lib;

    // Add books
    lib.add_book(Book("The Hobbit", "J.R.R. Tolkien", 1937, "Fantasy"));
    lib.add_book(Book("1984", "George Orwell", 1949, "Dystopian"));
    lib.add_book(Book("Animal Farm", "George Orwell", 1945, "Satire"));
    lib.add_book(Book("The Name of the Wind", "Patrick Rothfuss", 2007, "Fantasy"));
    lib.add_book(Book("Dune", "Frank Herbert", 1965, "Science Fiction"));

    // Print all books
    std::cout << "All books in library:\n";
    lib.print_all_books_iterator();

    // Find books by author
    std::cout << "\nBooks by George Orwell:\n";
    std::vector<Book> orwellBooks = lib.get_books_by_author("George Orwell");
    for (std::vector<Book>::const_iterator it = orwellBooks.begin(); it != orwellBooks.end(); ++it) {
        std::cout << it->display() << "\n";
    }

    // Books published after 1950
    std::cout << "\nBooks published after 1950:\n";
    std::vector<Book> recentBooks = lib.books_after_year(1950);
    for (std::vector<Book>::const_iterator it = recentBooks.begin(); it != recentBooks.end(); ++it) {
        std::cout << it->display() << "\n";
    }

    // Fantasy books sorted by year ascending
    std::cout << "\nFantasy books sorted by year ascending:\n";
    std::vector<std::string> fantasySorted = lib.pipeline_genre_sorted_by_year("Fantasy", true);
    for (std::vector<std::string>::const_iterator it = fantasySorted.begin(); it != fantasySorted.end(); ++it) {
        std::cout << *it << "\n";
    }

    // Remove a book
    std::cout << "\nRemoving '1984':\n";
    if (lib.remove_book("1984")) {
        std::cout << "'1984' removed successfully.\n";
    } else {
        std::cout << "'1984' not found.\n";
    }

    // Print all books after removal
    std::cout << "\nAll books after removal:\n";
    lib.print_all_books_iterator();

    // Search for a book by title within genre
    std::vector<Book> scifiBooks = lib.books_by_genre_simple("Science Fiction");
    std::vector<Book>::const_iterator it;
    bool found = false;
    for (it = scifiBooks.begin(); it != scifiBooks.end(); ++it) {
        if (it->title == "Dune") {
            found = true;
            break;
        }
    }

    if (found) {
        std::cout << "\nFound book: " << it->display() << "\n";
    } else {
        std::cout << "\nBook 'Dune' not found.\n";
    }

    return 0;
}
