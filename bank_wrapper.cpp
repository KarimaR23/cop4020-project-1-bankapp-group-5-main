
#include "bank.h"
#include <cstring>
#include <exception>
#include <string>

// Copy an error message safely into a Python-provided buffer.
void set_error(char* error, int error_size, const char* message) {
    if (error == nullptr || error_size <= 0) return;

    std::strncpy(error, message, error_size - 1);
    error[error_size - 1] = '\0';
}

extern "C" {

// Create a Bank object using a client CSV file.
Bank* bank_create(const char* filename, char* error, int error_size) {
    try {
        if (filename == nullptr || filename[0] == '\0') {
            set_error(error, error_size, "Please select a client file.");
            return nullptr;
        }

        return new Bank(filename);
    }
    catch (const std::exception& e) {
        set_error(error, error_size, e.what());
        return nullptr;
    }
    catch (...) {
        set_error(error, error_size, "Unknown error loading bank.");
        return nullptr;
    }
}

// Release the Bank object when Python closes it.
void bank_destroy(Bank* bank) {
    try {
        delete bank;
    }
    catch (...) {
        
    }
}

int bank_find_client(
    Bank* bank,
    const char* account,
    char* name,
    int name_size,
    char* ssn,
    int ssn_size,
    double* balance,
    char* error,
    int error_size
) {
    try {
        if (!bank || !account || !name || !ssn || !balance ||
            name_size <= 0 || ssn_size <= 0) {
            set_error(error, error_size, "Invalid search arguments.");
            return 0;
        }

        const Client* client = bank->find_client(account);

        if (!client) {
            set_error(error, error_size, "Account not found.");
            return 0;
        }

        // Return the client's name and SSN to Python.
        std::strncpy(name, client->client_name.c_str(), name_size - 1);
        name[name_size - 1] = '\0';

        std::strncpy(ssn, client->ssn.c_str(), ssn_size - 1);
        ssn[ssn_size - 1] = '\0';

        *balance = client->balance;
        return 1;

    } catch (const std::exception& e) {
        set_error(error, error_size, e.what());
        return 0;
    } catch (...) {
        set_error(error, error_size, "Unknown search error.");
        return 0;
    }
}

int bank_deposit(
    Bank* bank,
    const char* account,
    double amount,
    double* new_balance,
    char* error,
    int error_size
) {
    try {
        if (!bank || !account || !new_balance) {
            set_error(error, error_size, "Invalid deposit information.");
            return 0;
        }

        *new_balance = bank->deposit(account, amount);
        return 1;
    }
    catch (const std::exception& e) {
        set_error(error, error_size, e.what());
        return 0;
    }
    catch (...) {
        set_error(error, error_size, "Unknown deposit error.");
        return 0;
    }
}


int bank_withdraw(
    Bank* bank,
    const char* account,
    double amount,
    double* new_balance,
    char* error,
    int error_size
) {
    try {
        if (!bank || !account || !new_balance) {
            set_error(error, error_size, "Invalid withdrawal information.");
            return 0;
        }

        *new_balance = bank->withdraw(account, amount);
        return 1;
    }
    catch (const std::exception& e) {
        set_error(error, error_size, e.what());
        return 0;
    }
    catch (...) {
        set_error(error, error_size, "Unknown withdrawal error.");
        return 0;
    }
}

int bank_add_client(
    Bank* bank,
    const char* name,
    const char* ssn,
    const char* account,
    double balance,
    char* error,
    int error_size
) {
    try {
        if (!bank || !name || !ssn || !account) {
            set_error(error, error_size, "Invalid client information.");
            return 0;
        }

        Client client{
            name,
            ssn,
            account,
            balance
        };

        bank->add_new_client(client);
        return 1;
    }
    catch (const std::exception& e) {
        set_error(error, error_size, e.what());
        return 0;
    }
    catch (...) {
        set_error(error, error_size, "Unknown error adding client.");
        return 0;
    }
}
int bank_save(
    Bank* bank,
    const char* filename,
    char* error,
    int error_size
) {
    try {
        if (!bank || !filename || filename[0] == '\0') {
            set_error(error, error_size, "Invalid save information.");
            return 0;
        }

        bank->save_info(filename);
        return 1;
    }
    catch (const std::exception& e) {
        set_error(error, error_size, e.what());
        return 0;
    }
    catch (...) {
        set_error(error, error_size, "Unknown error saving client file.");
        return 0;
    }
}
}

