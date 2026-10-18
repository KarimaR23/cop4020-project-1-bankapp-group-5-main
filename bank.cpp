// bank.cpp

#include "bank.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <limits>

namespace {

// Remove leading/trailing whitespace 
std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t first = s.find_first_not_of(ws);
    if (first == std::string::npos) return "";
    std::size_t last = s.find_last_not_of(ws);
    return s.substr(first, last - first + 1);
}

std::string format_money(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

// Throws InvalidInputError unless every field of the client is usable.
// Commas/newlines are rejected because they would corrupt the CSV file.
void validate_client(const Client& c) {
    const char* bad = ",\r\n";
    if (c.client_name.empty())
        throw InvalidInputError("Client name must not be empty.");
    if (c.ssn.empty())
        throw InvalidInputError("SSN must not be empty.");
    if (c.bank_account_number.empty())
        throw InvalidInputError("Account number must not be empty.");
    if (c.client_name.find_first_of(bad) != std::string::npos ||
        c.ssn.find_first_of(bad) != std::string::npos ||
        c.bank_account_number.find_first_of(bad) != std::string::npos)
        throw InvalidInputError("Fields must not contain commas or line breaks.");
    if (!std::isfinite(c.balance) || c.balance < 0.0)
        throw InvalidInputError("Balance must be a number that is 0 or greater.");
}

double parse_balance(const std::string& text, int line_number) {
    try {
        std::size_t used = 0;
        double value = std::stod(text, &used);
        if (used != text.size()) throw std::invalid_argument("trailing characters");
        return value;
    } catch (const std::exception&) {
        throw FileError("Line " + std::to_string(line_number) +
                        ": invalid balance '" + text + "'.");
    }
}

// Turns one CSV line into a Client or throws FileError.
Client parse_client_line(const std::string& line, int line_number) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        std::size_t pos = line.find(',', start);
        if (pos == std::string::npos) {
            fields.push_back(trim(line.substr(start)));
            break;
        }
        fields.push_back(trim(line.substr(start, pos - start)));
        start = pos + 1;
    }
    if (fields.size() != 4)
        throw FileError("Line " + std::to_string(line_number) +
                        ": expected 4 comma-separated fields.");

    Client c{fields[0], fields[1], fields[2], parse_balance(fields[3], line_number)};
    try {
        validate_client(c);
    } catch (const InvalidInputError& e) {
        throw FileError("Line " + std::to_string(line_number) + ": " + e.what());
    }
    return c;
}

}  // namespace

// ---- Construction / Rule of Three ----------------------------------------

Bank::Bank() : num_clients(0), clients(nullptr) {}

Bank::Bank(const std::string& filename) : num_clients(0), clients(nullptr) {
    // load_from_file only assigns members after everything succeeded, so if it
    // throws, nothing has been allocated and nothing leaks.
    load_from_file(filename);
}

Bank::Bank(const Bank& other) : num_clients(0), clients(nullptr) {
    clients = clone_array(other.clients, other.num_clients);
    num_clients = other.num_clients;
}

Bank& Bank::operator=(const Bank& other) {
    if (this != &other) {
        // Copy first, then release: if copying throws, *this is unchanged.
        Client* copy = clone_array(other.clients, other.num_clients);
        delete[] clients;
        clients = copy;
        num_clients = other.num_clients;
    }
    return *this;
}

Bank::~Bank() {
    delete[] clients;
}

Client* Bank::clone_array(const Client* src, int n) {
    if (n <= 0 || src == nullptr) return nullptr;
    Client* copy = new Client[n];
    try {
        for (int i = 0; i < n; ++i) copy[i] = src[i];
    } catch (...) {
        delete[] copy;
        throw;
    }
    return copy;
}

// ---- File loading ----------------------------------------------------------

void Bank::load_from_file(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw FileError("Cannot open file '" + filename + "'.");

    std::vector<Client> loaded;  // temporary; the Bank keeps a raw heap array
    std::string line;
    int line_number = 0;
    while (std::getline(in, line)) {
        ++line_number;
        if (line_number == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line = line.substr(3);                       // strip UTF-8 BOM
        std::string stripped = trim(line);
        if (stripped.empty()) continue;                  // skip blank lines
        if (line_number == 1 && stripped.compare(0, 12, "client_name,") == 0)
            continue;                                    // tolerate a header row

        Client c = parse_client_line(line, line_number);
        for (const Client& existing : loaded) {
            if (existing.bank_account_number == c.bank_account_number)
                throw FileError("Line " + std::to_string(line_number) +
                                ": duplicate account number '" +
                                c.bank_account_number + "'.");
        }
        loaded.push_back(c);
    }
    if (in.bad()) throw FileError("Error while reading '" + filename + "'.");

    Client* array = nullptr;
    if (!loaded.empty()) {
        array = new Client[loaded.size()];
        try {
            for (std::size_t i = 0; i < loaded.size(); ++i) array[i] = loaded[i];
        } catch (...) {
            delete[] array;
            throw;
        }
    }
    delete[] clients;
    clients = array;
    num_clients = static_cast<int>(loaded.size());
}

// ---- Access / search -------------------------------------------------------

int Bank::get_num_clients() const { return num_clients; }

const Client* Bank::get_clients_info() const { return clients; }

int Bank::find_index(const std::string& account) const {
    for (int i = 0; i < num_clients; ++i) {
        if (clients[i].bank_account_number == account) return i;
    }
    return -1;
}

const Client* Bank::find_client(const std::string& account) const {
    int index = find_index(trim(account));
    return index < 0 ? nullptr : &clients[index];
}

// ---- Transactions ----------------------------------------------------------


double Bank::deposit(const std::string& account, double amount) {
    if (!std::isfinite(amount) || amount <= 0.0)
        throw InvalidInputError("Deposit amount must be greater than 0.");

    int index = find_index(trim(account));
    if (index < 0)
        throw AccountNotFoundError("Account '" + trim(account) + "' was not found.");

    // Prevent the account balance from overflowing.
    if (amount > std::numeric_limits<double>::max() - clients[index].balance)
        throw InvalidInputError("Deposit would exceed the maximum balance.");

    clients[index].balance += amount;
    return clients[index].balance;
}


double Bank::withdraw(const std::string& account, double amount) {
    if (!std::isfinite(amount) || amount <= 0.0)
        throw InvalidInputError("Withdrawal amount must be greater than 0.");
    int index = find_index(trim(account));
    if (index < 0)
        throw AccountNotFoundError("Account '" + trim(account) + "' was not found.");
    if (amount > clients[index].balance)
        throw InsufficientFundsError("Insufficient funds: balance is $" +
                                     format_money(clients[index].balance) +
                                     ", requested $" + format_money(amount) + ".");
    clients[index].balance -= amount;
    return clients[index].balance;
}

// ---- Adding clients --------------------------------------------------------

void Bank::add_new_client(const Client& client) {
    Client cleaned = client;
    cleaned.client_name = trim(cleaned.client_name);
    cleaned.ssn = trim(cleaned.ssn);
    cleaned.bank_account_number = trim(cleaned.bank_account_number);
    validate_client(cleaned);

    if (find_index(cleaned.bank_account_number) >= 0)
        throw DuplicateAccountError("Account number '" + cleaned.bank_account_number +
                                    "' already exists.");

    // Build the bigger array first; only swap it in once everything succeeded,
    // so a failure leaves the existing clients untouched.
    Client* bigger = new Client[num_clients + 1];
    try {
        for (int i = 0; i < num_clients; ++i) bigger[i] = clients[i];
        bigger[num_clients] = cleaned;
    } catch (...) {
        delete[] bigger;
        throw;
    }
    delete[] clients;
    clients = bigger;
    ++num_clients;
}

// ---- Saving ----------------------------------------------------------------

void Bank::save_info(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) throw FileError("Cannot open file '" + filename + "' for writing.");

    out << std::fixed << std::setprecision(2);
    for (int i = 0; i < num_clients; ++i) {
        out << clients[i].client_name << ',' << clients[i].ssn << ','
            << clients[i].bank_account_number << ',' << clients[i].balance << '\n';
    }
    out.close();
    if (out.fail()) throw FileError("Failed while writing '" + filename + "'.");
}
