#ifndef LIBRARY_H
#define LIBRARY_H

#include "Book.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <iterator>  // for std::back_inserter
#include <optional>

class Library {
private:
    std::vector<Book> books;

public:
    Library() {}

    // Add a book
    void add_book(const Book& book) {
        books.push_back(book);
    }

    // Remove a book by title (first match)
    bool remove_book(const std::string& title) {
        auto it = std::find_if(books.begin(), books.end(),
                               [&title](const Book& b) { return b.title == title; });
        if (it != books.end()) {
            books.erase(it);
            return true;
        }
        return false;
    }

    // Find a book by title and return optional<Book> (simulate optional for old C++)
    Book* find_book(const std::string& title) {
        auto it = std::find_if(books.begin(), books.end(),
                               [&title](Book& b) { return b.title == title; });
        if (it != books.end()) return &(*it);
        return nullptr;
    }

    // Return vector of books by author
    std::vector<Book> get_books_by_author(const std::string& author) const {
        std::vector<Book> result;
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            if (it->author == author) result.push_back(*it);
        }
        return result;
    }

    // Return vector of books by genre
    std::vector<Book> get_books_by_genre(const std::string& genre) const {
        std::vector<Book> result;
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            if (it->genre == genre) result.push_back(*it);
        }
        return result;
    }

    // Iterator traversal: print all books
    void print_all_books_iterator() const {
        std::cout << "All books (iterator traversal):\n";
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            std::cout << "  - " << it->display() << "\n";
        }
    }

    // Iterator-based search for books by author
    std::vector<std::vector<Book>::const_iterator> find_by_author_iter(const std::string& author) const {
        std::vector<std::vector<Book>::const_iterator> iters;
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            if (it->author == author) iters.push_back(it);
        }
        return iters;
    }

    // Filter books published after a given year
    std::vector<Book> books_after_year(int year) const {
        std::vector<Book> result;
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            if (it->year > year) result.push_back(*it);
        }
        return result;
    }

    // Filter books by genre
    std::vector<Book> books_by_genre_simple(const std::string& genre) const {
        std::vector<Book> result;
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            if (it->genre == genre) result.push_back(*it);
        }
        return result;
    }

    // Pipeline: filter by genre, sort by year, then transform to display strings
    std::vector<std::string> pipeline_genre_sorted_by_year(const std::string& genre, bool ascending = true) const {
        std::vector<Book> filtered;
        for (std::vector<Book>::const_iterator it = books.begin(); it != books.end(); ++it) {
            if (it->genre == genre) filtered.push_back(*it);
        }

        if (ascending) {
            std::sort(filtered.begin(), filtered.end(),
                      [](const Book& a, const Book& b) { return a.year < b.year; });
        } else {
            std::sort(filtered.begin(), filtered.end(),
                      [](const Book& a, const Book& b) { return a.year > b.year; });
        }

        std::vector<std::string> result;
        for (std::vector<Book>::const_iterator it = filtered.begin(); it != filtered.end(); ++it) {
            result.push_back(it->display());
        }

        return result;
    }

    // Remove books matching a predicate
    template<typename Pred>
    void remove_if(Pred pred) {
        books.erase(std::remove_if(books.begin(), books.end(), pred), books.end());
    }

    // Sort books by year ascending/descending
    void sort_by_year(bool ascending = true) {
        if (ascending) {
            std::sort(books.begin(), books.end(), [](const Book& a, const Book& b) { return a.year < b.year; });
        } else {
            std::sort(books.begin(), books.end(), [](const Book& a, const Book& b) { return a.year > b.year; });
        }
    }

    // Find book by title using iterator
    std::vector<Book>::const_iterator find_by_title_iter(const std::string& title) const {
        return std::find_if(books.begin(), books.end(),
                            [&title](const Book& b) { return b.title == title; });
    }

    // Add multiple books at once
    void add_books(const std::vector<Book>& more) {
        books.insert(books.end(), more.begin(), more.end());
    }
};

#endif // LIBRARY_H
