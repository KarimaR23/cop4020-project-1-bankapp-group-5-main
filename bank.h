// bank.h
// COP 4020 - Project 1, Part 1: Banking Application backend.

#ifndef BANK_H
#define BANK_H

#include <stdexcept>
#include <string>

// One bank customer. SSN and account number are strings on purpose: they are identifiers (may have leading zeros / dashes), never arithmetic.
struct Client {
    std::string client_name;
    std::string ssn;
    std::string bank_account_number;
    double balance;
};

// ---- Exceptions -----------------------------------------------------------
// All derive from BankError so main.cpp (or the C wrapper) can catch one type or a specific one.
class BankError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class InvalidInputError : public BankError {       // bad amount, empty field, ...
public:
    using BankError::BankError;
};
class AccountNotFoundError : public BankError {    // unknown account number
public:
    using BankError::BankError;
};
class InsufficientFundsError : public BankError {  // withdrawal > balance
public:
    using BankError::BankError;
};
class DuplicateAccountError : public BankError {   // account number already used
public:
    using BankError::BankError;
};
class FileError : public BankError {               // cannot open / read / write / parse
public:
    using BankError::BankError;
};

// ---- Bank -------------------------------------------------------------------
class Bank {
private:
    int num_clients;     // number of valid Client objects in the array
    Client* clients;     // heap array with exactly num_clients elements (nullptr if 0)

    // Helpers (no extra data members).
    int find_index(const std::string& account) const;      // -1 if missing
    void load_from_file(const std::string& filename);      // throws FileError
    static Client* clone_array(const Client* src, int n);  // deep copy helper

public:
    Bank();                                    // empty bank
    explicit Bank(const std::string& filename); // load clients from CSV; throws FileError
    Bank(const Bank& other);                   // deep copy
    Bank& operator=(const Bank& other);        // deep copy assignment
    ~Bank();                                   // releases heap memory

    // Access / search (nothing is printed).
    int get_num_clients() const;
    const Client* get_clients_info() const;                    // array of get_num_clients() items
    const Client* find_client(const std::string& account) const; // nullptr if not found

    // Transactions: return the new balance, throw on any problem.
    double deposit(const std::string& account, double amount);
    double withdraw(const std::string& account, double amount);

    // Adds a validated client; throws on invalid data or duplicate account.
    void add_new_client(const Client& client);

    // Writes every client as "name,ssn,account,balance" lines; throws FileError.
    void save_info(const std::string& filename) const;
};

#endif // BANK_H
