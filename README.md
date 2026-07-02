#SplitEase

SplitEase is a simple C++ console application for splitting expenses and tracking balances between users. It allows users to add shared expenses, view pending balances, settle payments, and save data using text files.

Features

• Create and manage users
• Add expenses with equal bill splitting
• View who owes you and whom you owe
• Settle pending payments
• Save and load data using text files

Concepts Used

• Object-Oriented Programming (OOP)
• File Handling
• STL (vector, unordered_map)
• Command Line Interface

Project Structure

main.cpp          - Main application
SplitEase.h       - Class definitions and implementations
users.txt         - User data
expenses.txt      - Expense records
ledger.txt        - Balance information

Compilation

g++ -std=c++17 main.cpp -o SplitEase

Run

Windows
SplitEase.exe

Linux / macOS
./SplitEase

Future Improvements

• Support unequal bill splitting
• Add group expenses
• Improve console interface
• Add expense history and editing
