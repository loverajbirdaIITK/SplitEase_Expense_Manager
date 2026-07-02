#ifndef SPLITEASE_H
#define SPLITEASE_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>

using namespace std;




class User {
private:
    int id;
    string name;

public:
    
    User() : id(0), name("") {}

    
    User(int id, const string& name) : id(id), name(name) {}

    
    int getId() const {
        return id;
    }

    
    string getName() const {
        return name;
    }

    
    void display() const {
        cout << "ID: " << id << " | Name: " << name << "\n";
    }
};





inline string getUserName(int id, const vector<User>& users) {
    for (const auto& u : users) {
        if (u.getId() == id) {
            return u.getName();
        }
    }
    return "Unknown";
}




class Expense {
public:
    string description;
    double amount;
    int payerID;
    vector<int> participants;

    
    Expense() : description(""), amount(0.0), payerID(0) {}

    
    Expense(const string& description, double amount, int payerID, const vector<int>& participants)
        : description(description), amount(amount), payerID(payerID), participants(participants) {}

    
    double calculateShare() const {
        if (participants.empty()) {
            return 0.0;
        }
        return amount / participants.size();
    }

    
    void display() const {
        cout << "Description: " << description << "\n";
        cout << "  Amount: ₹" << fixed << setprecision(2) << amount << "\n";
        cout << "  Paid by User ID: " << payerID << "\n";
        cout << "  Split among User IDs: ";
        for (size_t i = 0; i < participants.size(); ++i) {
            cout << participants[i];
            if (i + 1 < participants.size()) {
                cout << ", ";
            }
        }
        cout << "\n  Share: ₹" << fixed << setprecision(2) << calculateShare() << "\n";
    }
};




class Ledger {
private:
    unordered_map<int, unordered_map<int, double>> balances;

public:
    
    Ledger() {}

    
    void addExpense(int payerID, double amount, const vector<int>& participants) {
        if (participants.empty()) return;
        double share = amount / participants.size();
        for (int p : participants) {
            if (p != payerID) {
                addDebt(p, payerID, share);
            }
        }
    }

    
    void addDebt(int debtor, int creditor, double amount) {
        if (debtor == creditor || amount <= 0.0) return;
        double creditorOwesDebtor = balances[creditor][debtor];
        if (creditorOwesDebtor > 0.0) {
            if (amount >= creditorOwesDebtor) {
                balances[debtor][creditor] += (amount - creditorOwesDebtor);
                balances[creditor][debtor] = 0.0;
            } else {
                balances[creditor][debtor] -= amount;
            }
        } else {
            balances[debtor][creditor] += amount;
        }
    }

    
    void settleBill(int debtor, int creditor, double amount) {
        if (amount <= 0.0) return;
        double currentDebt = balances[debtor][creditor];
        if (currentDebt > 0.0) {
            if (amount >= currentDebt) {
                balances[debtor][creditor] = 0.0;
            } else {
                balances[debtor][creditor] -= amount;
            }
        }
    }

    
    void showWhoOwesMe(int userID, const vector<User>& users) const {
        bool found = false;
        for (const auto& outer : balances) {
            int debtorID = outer.first;
            const auto& innerMap = outer.second;
            auto it = innerMap.find(userID);
            if (it != innerMap.end() && it->second > 0.001) {
                string name = getUserName(debtorID, users);
                cout << "  - " << name << " (ID: " << debtorID << ") owes you ₹" << fixed << setprecision(2) << it->second << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "  No one owes you money.\n";
        }
    }

    
    void showWhomIOwe(int userID, const vector<User>& users) const {
        bool found = false;
        auto it = balances.find(userID);
        if (it != balances.end()) {
            for (const auto& inner : it->second) {
                int creditorID = inner.first;
                double amount = inner.second;
                if (amount > 0.001) {
                    string name = getUserName(creditorID, users);
                    cout << "  - You owe " << name << " (ID: " << creditorID << ") ₹" << fixed << setprecision(2) << amount << "\n";
                    found = true;
                }
            }
        }
        if (!found) {
            cout << "  You do not owe anyone money.\n";
        }
    }

    
    void clear() {
        balances.clear();
    }

    
    const unordered_map<int, unordered_map<int, double>>& getBalances() const {
        return balances;
    }
};




class StorageManager {
private:
    string usersFilePath;
    string expensesFilePath;
    string ledgerFilePath;

public:
    
    StorageManager(const string& usersFile = "users.txt",
                   const string& expensesFile = "expenses.txt",
                   const string& ledgerFile = "ledger.txt")
        : usersFilePath(usersFile), expensesFilePath(expensesFile), ledgerFilePath(ledgerFile) {}

    
    bool saveUsers(const vector<User>& users) const {
        ofstream file(usersFilePath);
        if (!file.is_open()) return false;
        for (const auto& u : users) {
            file << u.getId() << "," << u.getName() << "\n";
        }
        return true;
    }

    
    bool loadUsers(vector<User>& users) const {
        ifstream file(usersFilePath);
        if (!file.is_open()) return false;
        users.clear();
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, name;
            if (getline(ss, idStr, ',') && getline(ss, name)) {
                users.push_back(User(stoi(idStr), name));
            }
        }
        return true;
    }

    
    bool saveExpenses(const vector<Expense>& expenses) const {
        ofstream file(expensesFilePath);
        if (!file.is_open()) return false;
        for (const auto& e : expenses) {
            file << e.description << "|" << e.amount << "|" << e.payerID << "|";
            for (size_t i = 0; i < e.participants.size(); ++i) {
                file << e.participants[i];
                if (i + 1 < e.participants.size()) {
                    file << ",";
                }
            }
            file << "\n";
        }
        return true;
    }

    
    bool loadExpenses(vector<Expense>& expenses) const {
        ifstream file(expensesFilePath);
        if (!file.is_open()) return false;
        expenses.clear();
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string desc, amountStr, payerStr, participantsStr;
            if (getline(ss, desc, '|') &&
                getline(ss, amountStr, '|') &&
                getline(ss, payerStr, '|') &&
                getline(ss, participantsStr)) {
                vector<int> participants;
                stringstream ssParts(participantsStr);
                string partIdStr;
                while (getline(ssParts, partIdStr, ',')) {
                    if (!partIdStr.empty()) {
                        participants.push_back(stoi(partIdStr));
                    }
                }
                expenses.push_back(Expense(desc, stod(amountStr), stoi(payerStr), participants));
            }
        }
        return true;
    }

    
    bool saveLedger(const Ledger& ledger) const {
        ofstream file(ledgerFilePath);
        if (!file.is_open()) return false;
        for (const auto& outer : ledger.getBalances()) {
            int debtor = outer.first;
            for (const auto& inner : outer.second) {
                int creditor = inner.first;
                double amount = inner.second;
                if (amount > 0.001) {
                    file << debtor << "," << creditor << "," << amount << "\n";
                }
            }
        }
        return true;
    }

    
    bool loadLedger(Ledger& ledger) const {
        ifstream file(ledgerFilePath);
        if (!file.is_open()) return false;
        ledger.clear();
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string debtorStr, creditorStr, amountStr;
            if (getline(ss, debtorStr, ',') &&
                getline(ss, creditorStr, ',') &&
                getline(ss, amountStr)) {
                int debtor = stoi(debtorStr);
                int creditor = stoi(creditorStr);
                double amount = stod(amountStr);
                ledger.addDebt(debtor, creditor, amount);
            }
        }
        return true;
    }
};




class SplitEase {
private:
    vector<User> users;
    vector<Expense> expenses;
    Ledger ledger;
    StorageManager storageManager;




    
    bool userExists(int id) const {
        for (const auto& u : users) {
            if (u.getId() == id) return true;
        }
        return false;
    }

    
    int getValidIntInput(const string& prompt) const {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) {
                cin.ignore(10000, '\n');
                return val;
            }
            cout << "Invalid input. Please enter a valid integer.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    
    double getValidDoubleInput(const string& prompt) const {
        double val;
        while (true) {
            cout << prompt;
            if (cin >> val) {
                cin.ignore(10000, '\n');
                return val;
            }
            cout << "Invalid input. Please enter a valid number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    
    string getValidStringInput(const string& prompt) const {
        string val;
        cout << prompt;
        getline(cin, val);
        return val;
    }




    
    void displayMenu() const {
        cout << "\n=================================\n";
        cout << "      SplitEase Main Menu\n";
        cout << "=================================\n";
        cout << "1. Create User\n";
        cout << "2. Display Users\n";
        cout << "3. Search User\n";
        cout << "4. Add Expense\n";
        cout << "5. View Expense History\n";
        cout << "6. View Who Owes Me\n";
        cout << "7. View Whom I Owe\n";
        cout << "8. Settle Bill\n";
        cout << "9. Save Data\n";
        cout << "10. Load Data\n";
        cout << "11. Exit\n";
        cout << "=================================\n";
    }




    
    void handleCreateUser() {
        string name = getValidStringInput("Enter User Name: ");
        if (name.empty()) {
            cout << "User name cannot be empty.\n";
            return;
        }
        int newId = 1;
        for (const auto& u : users) {
            if (u.getId() >= newId) {
                newId = u.getId() + 1;
            }
        }
        users.push_back(User(newId, name));
        cout << "User '" << name << "' created successfully with ID: " << newId << "\n";
    }

    
    void handleDisplayUsers() const {
        if (users.empty()) {
            cout << "No users registered.\n";
            return;
        }
        cout << "\n--- Registered Users ---\n";
        for (const auto& u : users) {
            u.display();
        }
    }

    
    void handleSearchUser() const {
        if (users.empty()) {
            cout << "No users registered to search.\n";
            return;
        }
        int choice = getValidIntInput("Search by: 1. ID  2. Name\nChoice: ");
        if (choice == 1) {
            int id = getValidIntInput("Enter User ID: ");
            bool found = false;
            for (const auto& u : users) {
                if (u.getId() == id) {
                    u.display();
                    found = true;
                    break;
                }
            }
            if (!found) cout << "User with ID " << id << " not found.\n";
        } else if (choice == 2) {
            string name = getValidStringInput("Enter User Name: ");
            bool found = false;
            for (const auto& u : users) {
                if (u.getName() == name) {
                    u.display();
                    found = true;
                }
            }
            if (!found) cout << "No user found with name '" << name << "'.\n";
        } else {
            cout << "Invalid choice.\n";
        }
    }

    
    void handleAddExpense() {
        if (users.empty()) {
            cout << "Please create users first before adding expenses.\n";
            return;
        }
        string desc = getValidStringInput("Enter Expense Description (e.g., Dinner): ");
        if (desc.empty()) {
            cout << "Expense description cannot be empty.\n";
            return;
        }
        double amount = getValidDoubleInput("Enter Total Amount (₹): ");
        if (amount <= 0.0) {
            cout << "Amount must be positive.\n";
            return;
        }
        int payerID = getValidIntInput("Enter Payer User ID: ");
        if (!userExists(payerID)) {
            cout << "Payer ID " << payerID << " does not exist.\n";
            return;
        }
        int numParticipants = getValidIntInput("Enter number of participants: ");
        if (numParticipants <= 0) {
            cout << "Number of participants must be greater than 0.\n";
            return;
        }
        vector<int> participants;
        cout << "Enter the ID of each participant:\n";
        for (int i = 0; i < numParticipants; ++i) {
            while (true) {
                int pId = getValidIntInput("Participant " + to_string(i + 1) + " ID: ");
                if (!userExists(pId)) {
                    cout << "User with ID " << pId << " does not exist. Try again.\n";
                } else if (find(participants.begin(), participants.end(), pId) != participants.end()) {
                    cout << "User ID " << pId << " already added to this expense. Try again.\n";
                } else {
                    participants.push_back(pId);
                    break;
                }
            }
        }
        Expense exp(desc, amount, payerID, participants);
        expenses.push_back(exp);
        ledger.addExpense(payerID, amount, participants);
        cout << "Expense added successfully. Share per participant: ₹" << fixed << setprecision(2) << exp.calculateShare() << "\n";
    }

    
    void handleViewExpenseHistory() const {
        if (expenses.empty()) {
            cout << "No expenses recorded.\n";
            return;
        }
        cout << "\n--- Expense History ---\n";
        for (size_t i = 0; i < expenses.size(); ++i) {
            cout << i + 1 << ". ";
            expenses[i].display();
            cout << "\n";
        }
    }

    
    void handleViewWhoOwesMe() const {
        int userId = getValidIntInput("Enter your User ID: ");
        if (!userExists(userId)) {
            cout << "User ID " << userId << " does not exist.\n";
            return;
        }
        cout << "\n--- People Who Owe " << getUserName(userId, users) << " ---\n";
        ledger.showWhoOwesMe(userId, users);
    }

    
    void handleViewWhomIOwe() const {
        int userId = getValidIntInput("Enter your User ID: ");
        if (!userExists(userId)) {
            cout << "User ID " << userId << " does not exist.\n";
            return;
        }
        cout << "\n--- People " << getUserName(userId, users) << " Owes ---\n";
        ledger.showWhomIOwe(userId, users);
    }

    
    void handleSettleBill() {
        int debtor = getValidIntInput("Enter Debtor ID (person who owes money): ");
        if (!userExists(debtor)) {
            cout << "Debtor User ID " << debtor << " does not exist.\n";
            return;
        }
        int creditor = getValidIntInput("Enter Creditor ID (person who is owed money): ");
        if (!userExists(creditor)) {
            cout << "Creditor User ID " << creditor << " does not exist.\n";
            return;
        }
        if (debtor == creditor) {
            cout << "A user cannot owe money to themselves.\n";
            return;
        }
        double amount = getValidDoubleInput("Enter Settlement Amount (₹): ");
        if (amount <= 0.0) {
            cout << "Settlement amount must be positive.\n";
            return;
        }
        ledger.settleBill(debtor, creditor, amount);
        cout << "Settlement of ₹" << fixed << setprecision(2) << amount << " from "
             << getUserName(debtor, users) << " to " << getUserName(creditor, users) << " recorded successfully.\n";
    }

    
    void handleSaveData() {
        bool uSaved = storageManager.saveUsers(users);
        bool eSaved = storageManager.saveExpenses(expenses);
        bool lSaved = storageManager.saveLedger(ledger);
        if (uSaved && eSaved && lSaved) {
            cout << "All data saved successfully to users.txt, expenses.txt, and ledger.txt.\n";
        } else {
            cout << "Warning: Some data files could not be saved. Check file paths.\n";
        }
    }

    
    void handleLoadData() {
        bool uLoaded = storageManager.loadUsers(users);
        bool eLoaded = storageManager.loadExpenses(expenses);
        bool lLoaded = storageManager.loadLedger(ledger);
        if (uLoaded && eLoaded && lLoaded) {
            cout << "All data loaded successfully. Ledger restored from saved ledger state.\n";
        } else if (uLoaded && eLoaded) {
            cout << "Users and Expenses loaded. Reconstructing ledger balances by replaying expenses...\n";
            ledger.clear();
            for (const auto& exp : expenses) {
                ledger.addExpense(exp.payerID, exp.amount, exp.participants);
            }
        } else {
            cout << "No saved data found (or load failed). Starting with clean data.\n";
        }
    }

public:
    
    SplitEase() {}

    
    void run() {
        cout << "=========================================\n";
        cout << "      Welcome to SplitEase Application\n";
        cout << "=========================================\n";
        cout << "Attempting to auto-load saved data...\n";
        handleLoadData();

        int choice = 0;
        while (choice != 11) {
            displayMenu();
            choice = getValidIntInput("Enter Choice (1-11): ");
            cout << "\n";
            switch (choice) {
                case 1:
                    handleCreateUser();
                    break;
                case 2:
                    handleDisplayUsers();
                    break;
                case 3:
                    handleSearchUser();
                    break;
                case 4:
                    handleAddExpense();
                    break;
                case 5:
                    handleViewExpenseHistory();
                    break;
                case 6:
                    handleViewWhoOwesMe();
                    break;
                case 7:
                    handleViewWhomIOwe();
                    break;
                case 8:
                    handleSettleBill();
                    break;
                case 9:
                    handleSaveData();
                    break;
                case 10:
                    handleLoadData();
                    break;
                case 11:
                    cout << "Saving data before exit...\n";
                    handleSaveData();
                    cout << "Exiting SplitEase. Goodbye!\n";
                    break;
                default:
                    cout << "Invalid choice. Please select from 1 to 11.\n";
                    break;
            }
        }
    }
};

#endif
