#include "BankAccount.h"
#include <iostream>
#include <limits>

int main() {
    std::cout << "Creating accounts...\n\n";

    std::string name1, name2;
    double bal1 = 0.0, bal2 = 0.0;

    std::cout << "Enter account holder name for account 1: ";
    std::getline(std::cin, name1);
    if (name1.empty()) name1 = "Alice";
    std::cout << "Enter initial balance for " << name1 << ": ";
    while (!(std::cin >> bal1)) {
        std::cout << "Please enter a valid number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter account holder name for account 2: ";
    std::getline(std::cin, name2);
    if (name2.empty()) name2 = "Bob";
    std::cout << "Enter initial balance for " << name2 << ": ";
    while (!(std::cin >> bal2)) {
        std::cout << "Please enter a valid number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    BankAccount acc1(name1, bal1);
    BankAccount acc2(name2, bal2);

    std::cout << "\nInitial account info:\n";
    acc1.printAccountInfo();
    acc2.printAccountInfo();

    double amount = 0.0;
    std::cout << name1 << " deposits $";
    while (!(std::cin >> amount)) {
        std::cout << "Please enter a valid number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    acc1.deposit(amount);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << name2 << " withdraws $";
    while (!(std::cin >> amount)) {
        std::cout << "Please enter a valid number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    acc2.withdraw(amount);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\nUpdated account info:\n";
    acc1.printAccountInfo();
    acc2.printAccountInfo();

    std::cout << "Total active accounts: " << BankAccount::getTotalAccounts() << "\n\n";

    std::cout << "Entering block scope...\n";
    std::cout << "Total active accounts: " << BankAccount::getTotalAccounts() << "\n";

    char choice = 'n';
    std::cout << "Create a temporary account in block? (y/n): ";
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (choice == 'y' || choice == 'Y') {
        std::string tname;
        double tbal = 0.0;
        std::cout << "Enter temp account holder name: ";
        std::getline(std::cin, tname);
        if (tname.empty()) tname = "TempUser";
        std::cout << "Enter temp initial balance: ";
        while (!(std::cin >> tbal)) {
            std::cout << "Please enter a valid number: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        {
            BankAccount temp(tname, tbal);
            std::cout << "Leaving block scope...\n";
        }
    } else {
        std::cout << "Leaving block scope...\n";
    }

    std::cout << "Total active accounts after block: " << BankAccount::getTotalAccounts() << "\n\n";

    double tAmount = 0.0;
    std::cout << name1 << " transfers $";
    while (!(std::cin >> tAmount)) {
        std::cout << "Please enter a valid number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    acc1.transfer(acc2, tAmount);

    std::cout << "\nFinal account info:\n";
    acc1.printAccountInfo();
    acc2.printAccountInfo();

    std::cout << "Exiting program...\n";
    return 0;
}
