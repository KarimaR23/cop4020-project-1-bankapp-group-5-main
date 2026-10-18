# project1-BankApp
Build a banking application with a C++ backend and Python/Tkinter frontend, applying OOP, file handling, memory management, input validation, and cross-language integration.
# COP 4020 Project 1 — Banking Application

## Project Overview

This project is a banking application developed using C++ and Python. The C++ backend manages customer accounts and banking transactions, while the Python Tkinter frontend provides a graphical user interface (GUI).

The application uses Python's `ctypes` library to communicate with the compiled C++ shared library.

## Features

- Load customer information from a CSV file.
- Search for customers using their account numbers.
- Display customer information and account balances.
- Deposit money into an existing account.
- Withdraw money with insufficient-funds validation.
- Add new customers with duplicate-account validation.
- Save updated customer information to a CSV file.
- Display error messages for invalid operations.

## Technologies Used

- C++17
- Python 3
- Tkinter
- Python ctypes
- CSV file storage
- Git and GitHub
- Visual Studio Code

## Project Files

- `bank.h` — C++ banking class declarations.
- `bank.cpp` — C++ banking functionality.
- `bank_wrapper.cpp` — C-compatible interface connecting C++ and Python.
- `bank_app.py` — Python Tkinter graphical interface.
- `clients.csv` — Sample customer data.
- `main.cpp` — C++ backend testing or console application.
- `.gitignore` — Excludes generated files from Git tracking.

## How to Run the Application on macOS

### 1. Open the project folder

Open the project in Visual Studio Code and launch the integrated terminal.

### 2. Compile the C++ shared library

Run:


- g++ -std=c++17 -dynamiclib bank.cpp bank_wrapper.cpp -o libbank.dylib

This generates the shared library used by Python.

### 3. Start the Python application

Run:


python3 bank_app.py


The Banking Application window should appear.

### 4. Load customer information

Click **Load Client File** and select `clients.csv`.

### 5. Perform banking operations

Use the application buttons to search for customers, deposit funds, withdraw funds, and add customers.

### 6. Save updated information

Click **Save Client File** and choose a CSV filename, such as `clients_saved.csv`.

To reload saved information, restart the application and load the saved CSV file.

## Input Validation

The application includes error handling for:

- Invalid or missing account numbers.
- Negative or invalid transaction amounts.
- Insufficient account balances.
- Duplicate customer account numbers.
- Missing or invalid customer information.
- File loading and saving errors.

## Testing

The application has been tested for customer lookup, deposits, withdrawals, adding customers, and saving and reloading customer information.

A saved customer record was successfully reloaded and displayed with the correct account balance.

## Conclusion

This project demonstrates object-oriented programming in C++, graphical user interface development in Python, file handling, exception handling, and integration between two programming languages.