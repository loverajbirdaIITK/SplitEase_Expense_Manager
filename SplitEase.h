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

#if defined(__GNUC__) && (__GNUC__ < 8)
#include <direct.h>
namespace std {
    namespace filesystem {
        struct path {
            std::string p;
            path(const std::string& s) : p(s) {}
            std::string string() const { return p; }
        };
        inline path current_path() {
            char buf[1024];
            if (_getcwd(buf, sizeof(buf)) != nullptr) {
                return path(buf);
            }
            return path("");
        }
    }
}
#else
#include <filesystem>
#endif

using namespace std;

enum class ExpenseType {
    PERSONAL,
    GROUP
};

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
private:
    string description;
    double amount;
    int payerID;
    vector<int> participants;
    ExpenseType expenseType;
    string date;

public:
    Expense() 
        : description(""), amount(0.0), payerID(0), expenseType(ExpenseType::GROUP), date("Unknown") {}

    Expense(const string& description, double amount, int payerID, const vector<int>& participants, ExpenseType type = ExpenseType::GROUP, const string& date = "Unknown")
        : description(description), amount(amount), payerID(payerID), participants(participants), expenseType(type), date(date) {}

    const string& getDescription() const {
        return description;
    }

    double getAmount() const {
        return amount;
    }

    int getPayerID() const {
        return payerID;
    }

    const vector<int>& getParticipants() const {
        return participants;
    }

    ExpenseType getExpenseType() const {
        return expenseType;
    }

    const string& getDate() const {
        return date;
    }

    double calculateShare() const {
        if (participants.empty()) {
            return 0.0;
        }
        return amount / participants.size();
    }

    void display(const vector<User>& users) const {
        string typeStr = (getExpenseType() == ExpenseType::PERSONAL) ? "PERSONAL" : "GROUP";
        cout << "Description: " << getDescription() << "\n";
        cout << "  Date: " << getDate() << " | Type: " << typeStr << "\n";
        cout << "  Amount: ₹" << fixed << setprecision(2) << getAmount() << "\n";
        cout << "  Paid by: " << getUserName(getPayerID(), users) << "\n";
        if (getExpenseType() == ExpenseType::GROUP) {
            cout << "  Split among: ";
            const auto& participants = getParticipants();
            for (size_t i = 0; i < participants.size(); ++i) {
                cout << getUserName(participants[i], users);
                if (i + 1 < participants.size()) {
                    cout << ", ";
                }
            }
            cout << "\n  Share: ₹" << fixed << setprecision(2) << calculateShare() << "\n";
        }
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

    void displayLedger(const vector<User>& users) const {
        bool found = false;
        cout << "\nActive Ledger Balances:\n\n";
        for (const auto& outer : balances) {
            int debtorID = outer.first;
            for (const auto& inner : outer.second) {
                int creditorID = inner.first;
                double amount = inner.second;
                if (amount > 0.001) {
                    string debtorName = getUserName(debtorID, users);
                    string creditorName = getUserName(creditorID, users);
                    cout << "  - " << debtorName << " owes " 
                         << creditorName << " ₹" 
                         << fixed << setprecision(2) << amount << "\n";
                    found = true;
                }
            }
        }
        if (!found) {
            cout << "  No active debts in the system.\n";
        }
    }

    void clear() {
        balances.clear();
    }

    const unordered_map<int, unordered_map<int, double>>& getBalances() const {
        return balances;
    }
};

class BudgetManager {
private:
    unordered_map<int, unordered_map<string, double>> budgets;

    string toLower(string s) const {
        for (char& c : s) c = tolower(c);
        return s;
    }

    bool matchMonth(const string& m1, const string& m2) const {
        string a = toLower(m1);
        string b = toLower(m2);
        if (a.empty() || b.empty()) return false;
        if (a.length() >= 3 && b.length() >= 3) {
            return (a.rfind(b, 0) == 0 || b.rfind(a, 0) == 0);
        }
        return a == b;
    }

    string extractMonth(const string& date) const {
        stringstream ss(date);
        string word;
        while (ss >> word) {
            bool isAlpha = true;
            for (char c : word) {
                if (!isalpha(c)) {
                    isAlpha = false;
                    break;
                }
            }
            if (isAlpha && word.length() >= 3) {
                return word;
            }
        }
        return "Unknown";
    }

public:
    BudgetManager() {}

    void setBudget(int userID, const string& month, double budget) {
        budgets[userID][month] = budget;
    }

    double getBudget(int userID, const string& month) const {
        auto itUser = budgets.find(userID);
        if (itUser != budgets.end()) {
            auto itMonth = itUser->second.find(month);
            if (itMonth != itUser->second.end()) {
                return itMonth->second;
            }
        }
        return 0.0;
    }

    double calculateMonthlySpending(int userID, const string& targetMonth, const vector<Expense>& expenses) const {
        double total = 0.0;
        for (const auto& exp : expenses) {
            string expMonth = extractMonth(exp.getDate());
            if (matchMonth(expMonth, targetMonth)) {
                if (exp.getExpenseType() == ExpenseType::PERSONAL) {
                    if (exp.getPayerID() == userID) {
                        total += exp.getAmount();
                    }
                } else {
                    const auto& participants = exp.getParticipants();
                    auto it = find(participants.begin(), participants.end(), userID);
                    if (it != participants.end()) {
                        total += exp.calculateShare();
                    }
                }
            }
        }
        return total;
    }

    double calculateMoneyPaid(int userID, const string& targetMonth, const vector<Expense>& expenses) const {
        double total = 0.0;
        for (const auto& exp : expenses) {
            string expMonth = extractMonth(exp.getDate());
            if (exp.getPayerID() == userID && matchMonth(expMonth, targetMonth)) {
                total += exp.getAmount();
            }
        }
        return total;
    }

    void displayUserExpenseHistory(int userID, const vector<Expense>& expenses, const vector<User>& users, int typeFilter, int periodFilter, const string& targetMonth = "") const {
        bool found = false;
        int count = 1;

        for (const auto& exp : expenses) {
            bool involved = (exp.getPayerID() == userID);
            if (!involved) {
                for (int p : exp.getParticipants()) {
                    if (p == userID) {
                        involved = true;
                        break;
                    }
                }
            }
            if (!involved) continue;

            if (typeFilter == 1 && exp.getExpenseType() != ExpenseType::PERSONAL) continue;
            if (typeFilter == 2 && exp.getExpenseType() != ExpenseType::GROUP) continue;

            if (periodFilter == 1) {
                string expMonth = extractMonth(exp.getDate());
                if (!matchMonth(expMonth, targetMonth)) continue;
            }

            if (!found) {
                cout << "\nExpense History for " << getUserName(userID, users) << ":\n\n";
                found = true;
            }

            string typeStr = (exp.getExpenseType() == ExpenseType::PERSONAL) ? "PERSONAL" : "GROUP";
            double paidByMe = (exp.getPayerID() == userID) ? exp.getAmount() : 0.0;
            double myShare = (exp.getExpenseType() == ExpenseType::PERSONAL) ? exp.getAmount() : exp.calculateShare();

            cout << count++ << ". Date: " << exp.getDate() << " | Type: " << typeStr << " | Desc: " << exp.getDescription() << "\n";
            cout << "   Total Amount: ₹" << fixed << setprecision(2) << exp.getAmount() 
                 << " | Amount Paid By Me: ₹" << fixed << setprecision(2) << paidByMe 
                 << " | My Share: ₹" << fixed << setprecision(2) << myShare << "\n";
            
            cout << "   Paid By:\n" << getUserName(exp.getPayerID(), users) << "\n";
            if (exp.getExpenseType() == ExpenseType::GROUP) {
                cout << "   Participants:\n";
                const auto& participants = exp.getParticipants();
                for (size_t i = 0; i < participants.size(); ++i) {
                    cout << getUserName(participants[i], users);
                    if (i + 1 < participants.size()) cout << ", ";
                }
                cout << "\n";
            }
            cout << "\n";
        }

        if (!found) {
            cout << "No matching expenses found.\n";
        }
    }

    void displayFinancialDashboard(int userID, const string& targetMonth, const vector<Expense>& expenses, const Ledger& ledger, const vector<User>& users) const {
        double budget = getBudget(userID, targetMonth);
        double spent = calculateMonthlySpending(userID, targetMonth, expenses);
        double paid = calculateMoneyPaid(userID, targetMonth, expenses);
        double remaining = budget - spent;

        double moneyYouOwe = 0.0;
        const auto& balances = ledger.getBalances();
        auto it = balances.find(userID);
        if (it != balances.end()) {
            for (const auto& inner : it->second) {
                moneyYouOwe += inner.second;
            }
        }

        double moneyOwedToYou = 0.0;
        for (const auto& outer : balances) {
            int debtorID = outer.first;
            if (debtorID != userID) {
                auto itInner = outer.second.find(userID);
                if (itInner != outer.second.end()) {
                    moneyOwedToYou += itInner->second;
                }
            }
        }

        double netPosition = moneyOwedToYou - moneyYouOwe;

        cout << "\n             Monthly Finance Dashboard\n\n";
        cout << "User                : " << getUserName(userID, users) << "\n";
        cout << "Month               : " << targetMonth << "\n\n";
        cout << "Monthly Budget      : ₹" << fixed << setprecision(2) << budget << "\n";
        cout << "Personal Spending   : ₹" << fixed << setprecision(2) << spent << "\n";
        cout << "Money Paid          : ₹" << fixed << setprecision(2) << paid << "\n";
        cout << "Remaining Budget    : ₹" << fixed << setprecision(2) << remaining << "\n\n";
        cout << "Money You Owe       : ₹" << fixed << setprecision(2) << moneyYouOwe << "\n";
        cout << "Money Owed To You   : ₹" << fixed << setprecision(2) << moneyOwedToYou << "\n";
        
        if (netPosition >= 0.0) {
            cout << "Net Position        : +₹" << fixed << setprecision(2) << netPosition << "\n\n";
        } else {
            cout << "Net Position        : -₹" << fixed << setprecision(2) << -netPosition << "\n\n";
        }

        string statusStr = "Within Budget";
        if (budget > 0.0) {
            double usagePercent = (spent / budget) * 100.0;
            if (usagePercent > 100.0) {
                statusStr = "Budget Exceeded";
            } else if (usagePercent >= 80.0) {
                statusStr = "Approaching Budget";
            }
        } else {
            statusStr = "No Budget Set";
        }

        cout << "Budget Status       : " << statusStr << "\n\n";

        if (budget > 0.0 && spent > budget) {
            cout << "⚠ Warning: Monthly budget exceeded by ₹" << fixed << setprecision(2) << (spent - budget) << ".\n\n";
        }
    }

    void clear() {
        budgets.clear();
    }

    const unordered_map<int, unordered_map<string, double>>& getBudgets() const {
        return budgets;
    }
};

class SplitEase {
private:
    vector<User> users;
    vector<Expense> expenses;
    Ledger ledger;
    BudgetManager budgetManager;

    void displayAvailableUsers() const {
        cout << "\nAvailable Users:\n\n";
        for (const auto& u : users) {
            cout << u.getId() << ". " << u.getName() << "\n";
        }
        cout << "\n";
    }

    bool saveUsers() const {
        ofstream file("users.txt");
        if (!file.is_open()) return false;
        for (const auto& u : users) {
            file << u.getId() << "," << u.getName() << "\n";
        }
        return true;
    }

    bool loadUsers() {
        cout << "Loading users.txt...\n";
        ifstream file("users.txt");
        if (!file.is_open()) {
            cout << "Failed to open users.txt\n";
            return false;
        }
        cout << "users.txt opened successfully.\n";
        users.clear();
        string line;
        int count = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, name;
            if (getline(ss, idStr, ',') && getline(ss, name)) {
                users.push_back(User(stoi(idStr), name));
                count++;
            }
        }
        cout << "Loaded " << count << " users.\n";
        return true;
    }

    bool saveExpenses() const {
        ofstream file("expenses.txt");
        if (!file.is_open()) return false;
        for (const auto& e : expenses) {
            file << e.getDescription() << "|" << e.getAmount() << "|" << e.getPayerID() << "|";
            const auto& participants = e.getParticipants();
            for (size_t i = 0; i < participants.size(); ++i) {
                file << participants[i];
                if (i + 1 < participants.size()) {
                    file << ",";
                }
            }
            file << "|";
            if (e.getExpenseType() == ExpenseType::PERSONAL) {
                file << "PERSONAL";
            } else {
                file << "GROUP";
            }
            file << "|" << e.getDate() << "\n";
        }
        return true;
    }

    bool loadExpenses() {
        cout << "Loading expenses.txt...\n";
        ifstream file("expenses.txt");
        if (!file.is_open()) {
            cout << "Failed to open expenses.txt\n";
            return false;
        }
        cout << "expenses.txt opened successfully.\n";
        expenses.clear();
        string line;
        int count = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            vector<string> parts;
            string part;
            while (getline(ss, part, '|')) {
                parts.push_back(part);
            }
            if (parts.size() >= 4) {
                string desc = parts[0];
                double amount = stod(parts[1]);
                int payerID = stoi(parts[2]);
                string participantsStr = parts[3];
                
                ExpenseType type = ExpenseType::GROUP;
                string date = "Unknown";
                
                if (parts.size() >= 5) {
                    if (parts[4] == "PERSONAL" || parts[4] == "0") {
                        type = ExpenseType::PERSONAL;
                    } else {
                        type = ExpenseType::GROUP;
                    }
                }
                if (parts.size() >= 6) {
                    date = parts[5];
                }
                
                vector<int> participants;
                stringstream ssParts(participantsStr);
                string partIdStr;
                while (getline(ssParts, partIdStr, ',')) {
                    if (!partIdStr.empty()) {
                        participants.push_back(stoi(partIdStr));
                    }
                }
                expenses.push_back(Expense(desc, amount, payerID, participants, type, date));
                count++;
            }
        }
        cout << "Loaded " << count << " expenses.\n";
        return true;
    }

    bool saveLedger() const {
        ofstream file("ledger.txt");
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

    bool loadLedger() {
        cout << "Loading ledger.txt...\n";
        ifstream file("ledger.txt");
        if (!file.is_open()) {
            cout << "Failed to open ledger.txt\n";
            return false;
        }
        cout << "ledger.txt opened successfully.\n";
        ledger.clear();
        string line;
        int count = 0;
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
                count++;
            }
        }
        cout << "Loaded " << count << " ledger entries.\n";
        return true;
    }

    bool saveBudgets() const {
        ofstream file("budgets.txt");
        if (!file.is_open()) return false;
        for (const auto& userPair : budgetManager.getBudgets()) {
            int userID = userPair.first;
            for (const auto& monthPair : userPair.second) {
                file << userID << "," << monthPair.first << "," << monthPair.second << "\n";
            }
        }
        return true;
    }

    bool loadBudgets() {
        cout << "Loading budgets.txt...\n";
        ifstream file("budgets.txt");
        if (!file.is_open()) {
            cout << "Failed to open budgets.txt\n";
            return false;
        }
        cout << "budgets.txt opened successfully.\n";
        budgetManager.clear();
        string line;
        int count = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, monthStr, budgetStr;
            if (getline(ss, idStr, ',') && getline(ss, monthStr, ',') && getline(ss, budgetStr)) {
                budgetManager.setBudget(stoi(idStr), monthStr, stod(budgetStr));
                count++;
            }
        }
        if (count == 1) {
            cout << "Loaded 1 budget.\n";
        } else {
            cout << "Loaded " << count << " budgets.\n";
        }
        return true;
    }

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
        cout << "\n      SplitEase Main Menu\n\n";
        cout << "1. Add User\n";
        cout << "2. Add Expense\n";
        cout << "3. View Users\n";
        cout << "4. View Ledger\n";
        cout << "5. Settle Bill\n";
        cout << "6. View Expense History\n";
        cout << "7. Set Monthly Budget\n";
        cout << "8. View Monthly Dashboard\n";
        cout << "9. Save Data\n";
        cout << "10. Load Data\n";
        cout << "11. Exit\n\n";
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

    void handleAddExpense() {
        if (users.empty()) {
            cout << "Please create users first before adding expenses.\n";
            return;
        }

        int typeChoice = getValidIntInput("Select Expense Type:\n1. Personal Expense\n2. Group Expense\nChoice: ");
        ExpenseType type = ExpenseType::GROUP;
        if (typeChoice == 1) {
            type = ExpenseType::PERSONAL;
        } else if (typeChoice != 2) {
            cout << "Invalid choice. Defaulting to Group Expense.\n";
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

        displayAvailableUsers();
        int payerID = getValidIntInput("Enter Payer User ID: ");
        if (!userExists(payerID)) {
            cout << "Payer ID " << payerID << " does not exist.\n";
            return;
        }

        string date = getValidStringInput("Enter Date (e.g., \"12 Aug\"): ");
        if (date.empty()) {
            date = "Unknown";
        }

        vector<int> participants;
        if (type == ExpenseType::PERSONAL) {
            participants.push_back(payerID);
            Expense exp(desc, amount, payerID, participants, type, date);
            expenses.push_back(exp);
            cout << "Personal Expense added successfully.\n";
        } else {
            int numParticipants = getValidIntInput("Enter number of participants: ");
            if (numParticipants <= 0) {
                cout << "Number of participants must be greater than 0.\n";
                return;
            }
            displayAvailableUsers();
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
            Expense exp(desc, amount, payerID, participants, type, date);
            expenses.push_back(exp);
            ledger.addExpense(payerID, amount, participants);
            cout << "Group Expense added successfully. Share per participant: ₹" 
                 << fixed << setprecision(2) << exp.calculateShare() << "\n";
        }
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

    void handleViewLedger() const {
        ledger.displayLedger(users);
    }

    void handleSettleBill() {
        displayAvailableUsers();
        int debtor = getValidIntInput("Enter Debtor ID (person who owes money): ");
        if (!userExists(debtor)) {
            cout << "Debtor User ID " << debtor << " does not exist.\n";
            return;
        }
        displayAvailableUsers();
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

    void handleViewExpenseHistory() const {
        if (expenses.empty()) {
            cout << "No expenses recorded.\n";
            return;
        }

        displayAvailableUsers();
        int userId = getValidIntInput("Enter your User ID: ");
        if (!userExists(userId)) {
            cout << "User ID " << userId << " does not exist.\n";
            return;
        }

        int typeFilter = getValidIntInput("Filter by Expense Type:\n1. Personal Expenses\n2. Group Expenses\n3. All Expenses\nChoice: ");
        int periodFilter = getValidIntInput("Filter by Period:\n1. Target Month\n2. Complete History\nChoice: ");
        
        string month = "";
        if (periodFilter == 1) {
            month = getValidStringInput("Enter Target Month (e.g. \"August\"): ");
        }

        budgetManager.displayUserExpenseHistory(userId, expenses, users, typeFilter, periodFilter, month);
    }

    void handleSetMonthlyBudget() {
        displayAvailableUsers();
        int userId = getValidIntInput("Enter User ID: ");
        if (!userExists(userId)) {
            cout << "User ID " << userId << " does not exist.\n";
            return;
        }
        string month = getValidStringInput("Enter Month (e.g. \"August\"): ");
        if (month.empty()) {
            cout << "Month cannot be empty.\n";
            return;
        }
        double amount = getValidDoubleInput("Enter Budget Amount (₹): ");
        if (amount < 0.0) {
            cout << "Budget cannot be negative.\n";
            return;
        }
        budgetManager.setBudget(userId, month, amount);
        cout << "Monthly budget of ₹" << fixed << setprecision(2) << amount 
             << " for " << month << " set successfully for user " 
             << getUserName(userId, users) << ".\n";
    }

    void handleViewMonthlyDashboard() const {
        displayAvailableUsers();
        int userId = getValidIntInput("Enter User ID: ");
        if (!userExists(userId)) {
            cout << "User ID " << userId << " does not exist.\n";
            return;
        }
        string month = getValidStringInput("Enter Month (e.g. \"August\"): ");
        if (month.empty()) {
            cout << "Month cannot be empty.\n";
            return;
        }
        budgetManager.displayFinancialDashboard(userId, month, expenses, ledger, users);
    }

    void handleSaveData() {
        bool uSaved = saveUsers();
        bool eSaved = saveExpenses();
        bool lSaved = saveLedger();
        bool bSaved = saveBudgets();
        if (uSaved && eSaved && lSaved && bSaved) {
            cout << "All data saved successfully to users.txt, expenses.txt, ledger.txt, and budgets.txt.\n";
        } else {
            cout << "Warning: Some data files could not be saved. Check file paths.\n";
        }
    }

    void handleLoadData() {
        if (loadUsers()) {
            cout << "Users loaded successfully.\n";
        } else {
            cout << "No saved users found.\n";
        }

        if (loadExpenses()) {
            cout << "Expenses loaded successfully.\n";
        } else {
            cout << "No saved expenses found.\n";
        }

        if (loadLedger()) {
            cout << "Ledger loaded successfully.\n";
        } else {
            cout << "Ledger file is missing.\n";
            ledger.clear();
            for (const auto& exp : expenses) {
                if (exp.getExpenseType() == ExpenseType::GROUP) {
                    ledger.addExpense(exp.getPayerID(), exp.getAmount(), exp.getParticipants());
                }
            }
            cout << "Ledger reconstructed from group expenses.\n";
        }

        if (loadBudgets()) {
            cout << "Budgets loaded successfully.\n";
        } else {
            budgetManager.clear();
            cout << "Warning: No saved budgets were found.\n";
        }
    }

public:
    SplitEase() {}

    void run() {
        cout << "\n      Welcome to SplitEase Application\n\n";
        cout << "Current Working Directory: " << std::filesystem::current_path().string() << "\n";
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
                    handleAddExpense();
                    break;
                case 3:
                    handleDisplayUsers();
                    break;
                case 4:
                    handleViewLedger();
                    break;
                case 5:
                    handleSettleBill();
                    break;
                case 6:
                    handleViewExpenseHistory();
                    break;
                case 7:
                    handleSetMonthlyBudget();
                    break;
                case 8:
                    handleViewMonthlyDashboard();
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
