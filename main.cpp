    // main.cpp
// Console test driver for the Bank backend (Part 1). Not the GUI.

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <limits>
#include "bank.h"

namespace {

int passes = 0;
int failures = 0;

void check(const std::string& label, bool ok) {
    std::cout << (ok ? "  [PASS] " : "  [FAIL] ") << label << "\n";
    ok ? ++passes : ++failures;
}

// Runs `action` and expects the backend to reject it with a BankError.
template <typename Func>
void expect_error(const std::string& label, Func action) {
    try {
        action();
        std::cout << "  [FAIL] " << label << " was accepted but should be rejected\n";
        ++failures;
    } catch (const BankError& e) {
        std::cout << "  [PASS] " << label << " rejected: " << e.what() << "\n";
        ++passes;
    }S
}

bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

void print_client(const Client& c) {
    std::cout << "    " << c.client_name << " | " << c.ssn << " | "
              << c.bank_account_number << " | $" << std::fixed
              << std::setprecision(2) << c.balance << "\n";
}

void heading(const std::string& text) { std::cout << "\n=== " << text << " ===\n"; }

}  // namespace

int main() {
    const std::string kAccount = "0001234567";  

    try {
        // 1. Load a file and show how many clients were read.
        heading("1. Load file");
        Bank bank("clients.csv");
        std::cout << "  Clients loaded: " << bank.get_num_clients() << "\n";
        check("3 clients loaded", bank.get_num_clients() == 3);
        const Client* all = bank.get_clients_info();
        for (int i = 0; i < bank.get_num_clients(); ++i) print_client(all[i]);

        expect_error("loading a missing file", [] { Bank bad("no_such_file.csv"); });

        // 2. Find an existing and a missing account.
        heading("2. Find client");
        const Client* found = bank.find_client(kAccount);
        check("existing account found", found != nullptr);
        if (found) print_client(*found);
        check("missing account returns nullptr", bank.find_client("9999999999") == nullptr);

        // 3. Valid deposit and withdrawal.
        heading("3. Valid deposit / withdrawal");
        double bal = bank.deposit(kAccount, 100.00);
        std::cout << "  After deposit of 100.00: " << bal << "\n";
        check("balance is 1350.75", near(bal, 1350.75));
        bal = bank.withdraw(kAccount, 50.25);
        std::cout << "  After withdrawal of 50.25: " << bal << "\n";
        check("balance is 1300.50", near(bal, 1300.50));

        // 4. Invalid amounts and insufficient funds.
        heading("4. Invalid transactions");
        expect_error("deposit of -5", [&] { bank.deposit(kAccount, -5.0); });
        expect_error("deposit of 0", [&] { bank.deposit(kAccount, 0.0); });
        expect_error("deposit overflow", [&] { Bank overflowBank;
    overflowBank.add_new_client(
        Client{"Overflow Test", "111-22-3333",
               "0009999999", std::numeric_limits<double>::max()}
    );
    overflowBank.deposit("0009999999",
                         std::numeric_limits<double>::max());
});

        expect_error("withdraw of 0", [&] { bank.withdraw(kAccount, 0.0); });
        expect_error("withdraw of 1,000,000 (insufficient funds)",
                     [&] { bank.withdraw(kAccount, 1000000.0); });
        expect_error("deposit to unknown account", [&] { bank.deposit("9999999999", 10.0); });
        check("balance unchanged after rejected operations",
              near(bank.find_client(kAccount)->balance, 1300.50));

        // 5. Add a new client and a duplicate.
        heading("5. Add client");
        bank.add_new_client(Client{"Taylor Smith", "222-33-4444", "0005678901", 300.00});
        check("4 clients after add", bank.get_num_clients() == 4);

        check("old clients preserved", bank.find_client("0007654321") != nullptr &&
                                       bank.find_client("0005550001") != nullptr &&
                                       near(bank.find_client(kAccount)->balance, 1300.50));

        expect_error("duplicate account number", [&] {
            bank.add_new_client(Client{"Copy Cat", "000-00-0000", "0005678901", 1.0});
        });
        expect_error("empty client name", [&] {
            bank.add_new_client(Client{"", "000-00-0000", "0006789012", 1.0});
        });
        
expect_error("negative starting balance", [&] {
    bank.add_new_client(
        Client{"Invalid Balance", "111-22-3333",
               "0008888888", -100.00}
    );
});

expect_error("empty account number", [&] {
    bank.add_new_client(
        Client{"Missing Account", "111-22-3333",
               "", 100.00}
    );
});

        
        check("still 4 clients", bank.get_num_clients() == 4);

        // 6. Save to a new file, then reload it to prove it round-trips.
        heading("6. Save and reload");
        bank.save_info("clients_saved.csv");
        Bank reloaded("clients_saved.csv");
        check("reloaded has 4 clients", reloaded.get_num_clients() == 4);
        check("reloaded balances match", near(reloaded.find_client(kAccount)->balance, 1300.50) &&
                                         near(reloaded.find_client("0005678901")->balance, 300.00));
        expect_error("saving to an unwritable path",
                     [&] { bank.save_info("/no_such_folder/out.csv"); });

        // 7. Deep copy behavior.
        heading("7. Deep copy");
        Bank copy(bank);                       // copy constructor
        copy.deposit(kAccount, 1000.0);
        std::cout << "  original: " << bank.find_client(kAccount)->balance
                  << "   copy: " << copy.find_client(kAccount)->balance << "\n";
        check("copy-constructed Bank is independent",
              near(bank.find_client(kAccount)->balance, 1300.50) &&
              near(copy.find_client(kAccount)->balance, 2300.50));

        Bank assigned;
        assigned = bank;                       // copy assignment
        assigned.withdraw(kAccount, 300.50);
        check("copy-assigned Bank is independent",
              near(bank.find_client(kAccount)->balance, 1300.50) &&
              near(assigned.find_client(kAccount)->balance, 1000.00));
        assigned.add_new_client(Client{"Only In Copy", "333-44-5555", "0007777777", 5.0});
        check("adding to a copy does not affect the original",
              bank.get_num_clients() == 4 && assigned.get_num_clients() == 5);

        Bank& self = assigned;
        assigned = self;                       // self-assignment must be safe
        check("self-assignment is safe", assigned.get_num_clients() == 5);

    } catch (const std::exception& e) {
        std::cout << "UNEXPECTED ERROR: " << e.what() << "\n";
        return 2;
    }

    std::cout << "\nSummary: " << passes << " passed, " << failures << " failed\n";
    return failures == 0 ? 0 : 1;
}
