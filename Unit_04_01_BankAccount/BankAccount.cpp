#include "BankAccount.h"
#include <iostream>

// Initialize static members
unsigned long BankAccount::nextAccountId = 1001;
unsigned long BankAccount::activeAccounts = 0;

std::string BankAccount::generateAccountNumber() {
    return std::to_string(nextAccountId++);
}

BankAccount::BankAccount()
    : accountNumber(generateAccountNumber()), ownerName("Unknown"), balance(0.0) {
    ++activeAccounts;
}

BankAccount::BankAccount(const std::string& name, double initialBalance)
    : accountNumber(generateAccountNumber()), ownerName(name), balance(initialBalance) {
    ++activeAccounts;
}

BankAccount::~BankAccount() {
    if (activeAccounts > 0) --activeAccounts;
    std::cout << "Destructor called for account " << accountNumber << "\n";
}

void BankAccount::deposit(double amount) {
    if (amount > 0) {
        balance += amount;
        // success message handled by caller to match sample output
    } else {
        std::cout << "Invalid deposit amount.\n";
    }
}

void BankAccount::withdraw(double amount) {
    if (amount > 0 && amount <= balance) {
        balance -= amount;
        // success message handled by caller to match sample output
    } else {
        std::cout << "Invalid withdrawal amount.\n";
    }
}

void BankAccount::displayBalance() const {
    std::cout << "Owner: " << ownerName << "\n";
    std::cout << "Account Number: " << accountNumber << "\n";
    std::cout << "Current Balance: " << balance << "\n";
}

const std::string& BankAccount::getOwnerName() const { return ownerName; }
const std::string& BankAccount::getAccountNumber() const { return accountNumber; }
double BankAccount::getBalance() const { return balance; }

unsigned long BankAccount::getTotalAccounts() { return activeAccounts; }

void BankAccount::printAccountInfo() const {
    // Print in the requested sample format
    std::cout << "Account Holder: " << ownerName << "\n";
    std::cout << "Account Number: " << accountNumber << "\n";
    // format balance as dollars with no fractional part
    std::cout.setf(std::ios::fixed);
    std::cout.precision(0);
    std::cout << "Balance: $" << balance << "\n\n";
    // restore formatting (not strictly necessary here)
    std::cout.unsetf(std::ios::fixed);
}

bool BankAccount::transfer(BankAccount& toAccount, double amount) {
    if (amount <= 0) {
        std::cout << "Transfer failed: amount must be positive.\n";
        return false;
    }
    if (amount > balance) {
        std::cout << "Transfer failed: insufficient funds.\n";
        return false;
    }
    // perform transfer
    balance -= amount;
    toAccount.balance += amount;
    // Print a human-friendly transfer message using owner names and dollar formatting
    std::cout.setf(std::ios::fixed);
    std::cout.precision(0);
    std::cout << "Transferred $" << amount << " from " << ownerName
              << " to " << toAccount.ownerName << "\n";
    std::cout.unsetf(std::ios::fixed);
    return true;
}
