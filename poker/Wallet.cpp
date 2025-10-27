#include "Wallet.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>

Wallet::Wallet() : balance(1000), ownerName("Unknown"), filename("wallet.txt"), 
                   autoSave(false), maxBalance(100000), minBalance(0) {
    transactionHistory.clear();
}

Wallet::Wallet(const std::string& owner, int initialBalance) 
    : balance(initialBalance), ownerName(owner), filename("wallet.txt"), 
      autoSave(false), maxBalance(100000), minBalance(0) {
    transactionHistory.clear();
    addTransaction(TransactionType::DEPOSIT, initialBalance, "Initial balance");
}

Wallet::Wallet(const std::string& owner, int initialBalance, const std::string& file) 
    : balance(initialBalance), ownerName(owner), filename(file), 
      autoSave(false), maxBalance(100000), minBalance(0) {
    transactionHistory.clear();
    addTransaction(TransactionType::DEPOSIT, initialBalance, "Initial balance");
}

bool Wallet::deposit(int amount, const std::string& description) {
    if (amount <= 0) {
        std::cout << "Ошибка: Сумма пополнения должна быть положительной" << std::endl;
        return false;
    }
    
    if (!isWithinLimits(balance + amount)) {
        std::cout << "Ошибка: Пополнение превысит максимальный лимит баланса" << std::endl;
        return false;
    }
    
    balance += amount;
    addTransaction(TransactionType::DEPOSIT, amount, description);
    
    if (autoSave) {
        saveToFile();
    }
    
    return true;
}

bool Wallet::withdraw(int amount, const std::string& description) {
    if (amount <= 0) {
        std::cout << "Ошибка: Сумма снятия должна быть положительной" << std::endl;
        return false;
    }
    
    if (!canAfford(amount)) {
        std::cout << "Ошибка: Недостаточно средств для снятия" << std::endl;
        return false;
    }
    
    if (!isWithinLimits(balance - amount)) {
        std::cout << "Ошибка: Снятие приведет к превышению минимального лимита баланса" << std::endl;
        return false;
    }
    
    balance -= amount;
    addTransaction(TransactionType::WITHDRAWAL, amount, description);
    
    if (autoSave) {
        saveToFile();
    }
    
    return true;
}

bool Wallet::canAfford(int amount) const {
    return amount <= balance;
}

int Wallet::getBalance() const {
    return balance;
}

void Wallet::setBalance(int amount) {
    if (amount >= 0 && isWithinLimits(amount)) {
        balance = amount;
    }
}

bool Wallet::placeBet(int amount, const std::string& gameId) {
    if (!canAfford(amount)) {
        return false;
    }
    
    balance -= amount;
    std::string description = "Bet placed";
    if (!gameId.empty()) {
        description += " (Game: " + gameId + ")";
    }
    addTransaction(TransactionType::BET, amount, description);
    
    if (autoSave) {
        saveToFile();
    }
    
    return true;
}

void Wallet::winBet(int amount, const std::string& gameId) {
    balance += amount;
    std::string description = "Bet won";
    if (!gameId.empty()) {
        description += " (Game: " + gameId + ")";
    }
    addTransaction(TransactionType::WIN, amount, description);
    
    if (autoSave) {
        saveToFile();
    }
}

void Wallet::loseBet(int amount, const std::string& gameId) {
    std::string description = "Bet lost";
    if (!gameId.empty()) {
        description += " (Game: " + gameId + ")";
    }
    addTransaction(TransactionType::LOSS, amount, description);
    
    if (autoSave) {
        saveToFile();
    }
}

void Wallet::refundBet(int amount, const std::string& gameId) {
    balance += amount;
    std::string description = "Bet refunded";
    if (!gameId.empty()) {
        description += " (Game: " + gameId + ")";
    }
    addTransaction(TransactionType::REFUND, amount, description);
    
    if (autoSave) {
        saveToFile();
    }
}

void Wallet::addBonus(int amount, const std::string& reason) {
    if (amount > 0) {
        balance += amount;
        addTransaction(TransactionType::BONUS, amount, reason);
        
        if (autoSave) {
            saveToFile();
        }
    }
}

void Wallet::setMaxBalance(int max) {
    maxBalance = max;
}

void Wallet::setMinBalance(int min) {
    minBalance = min;
}

int Wallet::getMaxBalance() const {
    return maxBalance;
}

int Wallet::getMinBalance() const {
    return minBalance;
}

bool Wallet::isWithinLimits(int amount) const {
    return amount >= minBalance && amount <= maxBalance;
}

std::vector<Transaction> Wallet::getTransactionHistory() const {
    return transactionHistory;
}

std::vector<Transaction> Wallet::getTransactionsByType(TransactionType type) const {
    std::vector<Transaction> filtered;
    for (const auto& transaction : transactionHistory) {
        if (transaction.type == type) {
            filtered.push_back(transaction);
        }
    }
    return filtered;
}

std::vector<Transaction> Wallet::getRecentTransactions(int count) const {
    std::vector<Transaction> recent = transactionHistory;
    if (recent.size() > count) {
        recent.erase(recent.begin(), recent.end() - count);
    }
    return recent;
}

int Wallet::getTotalDeposits() const {
    int total = 0;
    for (const auto& transaction : transactionHistory) {
        if (transaction.type == TransactionType::DEPOSIT) {
            total += transaction.amount;
        }
    }
    return total;
}

int Wallet::getTotalWithdrawals() const {
    int total = 0;
    for (const auto& transaction : transactionHistory) {
        if (transaction.type == TransactionType::WITHDRAWAL) {
            total += transaction.amount;
        }
    }
    return total;
}

int Wallet::getTotalWinnings() const {
    int total = 0;
    for (const auto& transaction : transactionHistory) {
        if (transaction.type == TransactionType::WIN) {
            total += transaction.amount;
        }
    }
    return total;
}

int Wallet::getTotalLosses() const {
    int total = 0;
    for (const auto& transaction : transactionHistory) {
        if (transaction.type == TransactionType::LOSS) {
            total += transaction.amount;
        }
    }
    return total;
}

double Wallet::getWinRate() const {
    int wins = 0;
    int losses = 0;
    
    for (const auto& transaction : transactionHistory) {
        if (transaction.type == TransactionType::WIN) {
            wins++;
        } else if (transaction.type == TransactionType::LOSS) {
            losses++;
        }
    }
    
    int totalGames = wins + losses;
    if (totalGames == 0) return 0.0;
    
    return static_cast<double>(wins) / totalGames * 100.0;
}

int Wallet::getNetProfit() const {
    return getTotalWinnings() - getTotalLosses();
}

std::map<std::string, int> Wallet::getDailyStats() const {
    std::map<std::string, int> dailyStats;
    
    for (const auto& transaction : transactionHistory) {
        auto time_t = std::chrono::system_clock::to_time_t(transaction.timestamp);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d");
        std::string date = oss.str();
        
        dailyStats[date] += transaction.amount;
    }
    
    return dailyStats;
}

std::map<std::string, int> Wallet::getMonthlyStats() const {
    std::map<std::string, int> monthlyStats;
    
    for (const auto& transaction : transactionHistory) {
        auto time_t = std::chrono::system_clock::to_time_t(transaction.timestamp);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m");
        std::string month = oss.str();
        
        monthlyStats[month] += transaction.amount;
    }
    
    return monthlyStats;
}

void Wallet::saveToFile() const {
    saveToFile(filename);
}

void Wallet::saveToFile(const std::string& file) const {
    std::ofstream outFile(file);
    if (!outFile.is_open()) {
        std::cout << "Ошибка: Не удалось открыть файл " << file << " для записи" << std::endl;
        return;
    }
    
    outFile << "Owner: " << ownerName << std::endl;
    outFile << "Balance: " << balance << std::endl;
    outFile << "MaxBalance: " << maxBalance << std::endl;
    outFile << "MinBalance: " << minBalance << std::endl;
    outFile << "AutoSave: " << (autoSave ? "true" : "false") << std::endl;
    outFile << std::endl;
    
    outFile << "Type,Amount,Description,Timestamp,BalanceAfter" << std::endl;
    for (const auto& transaction : transactionHistory) {
        outFile << getTransactionTypeString(transaction.type) << ","
                << transaction.amount << ","
                << transaction.description << ","
                << getTimestampString(transaction.timestamp) << ","
                << transaction.balanceAfter << std::endl;
    }
    
    outFile.close();
}

void Wallet::loadFromFile() {
    loadFromFile(filename);
}

void Wallet::loadFromFile(const std::string& file) {
    std::ifstream inFile(file);
    if (!inFile.is_open()) {
        std::cout << "Ошибка: Не удалось открыть файл " << file << " для чтения" << std::endl;
        return;
    }
    
    std::string line;
    std::getline(inFile, line); // Owner
    if (line.find("Owner: ") == 0) {
        ownerName = line.substr(7);
    }
    
    std::getline(inFile, line); // Balance
    if (line.find("Balance: ") == 0) {
        balance = std::stoi(line.substr(9));
    }
    
    std::getline(inFile, line); // MaxBalance
    if (line.find("MaxBalance: ") == 0) {
        maxBalance = std::stoi(line.substr(12));
    }
    
    std::getline(inFile, line); // MinBalance
    if (line.find("MinBalance: ") == 0) {
        minBalance = std::stoi(line.substr(12));
    }
    
    std::getline(inFile, line); // AutoSave
    if (line.find("AutoSave: ") == 0) {
        autoSave = (line.substr(10) == "true");
    }
    
    std::getline(inFile, line); // Empty line
    std::getline(inFile, line); // Header
    
    transactionHistory.clear();
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 5) {
            TransactionType type;
            std::string typeStr = tokens[0];
            if (typeStr == "DEPOSIT") type = TransactionType::DEPOSIT;
            else if (typeStr == "WITHDRAWAL") type = TransactionType::WITHDRAWAL;
            else if (typeStr == "BET") type = TransactionType::BET;
            else if (typeStr == "WIN") type = TransactionType::WIN;
            else if (typeStr == "LOSS") type = TransactionType::LOSS;
            else if (typeStr == "REFUND") type = TransactionType::REFUND;
            else if (typeStr == "BONUS") type = TransactionType::BONUS;
            else continue;
            
            Transaction transaction(type, std::stoi(tokens[1]), tokens[2], std::stoi(tokens[4]));
            transactionHistory.push_back(transaction);
        }
    }
    
    inFile.close();
}

void Wallet::setAutoSave(bool enable) {
    autoSave = enable;
}

void Wallet::setFilename(const std::string& file) {
    filename = file;
}

void Wallet::displayBalance() const {
    std::cout << "\n=== БАЛАНС КОШЕЛЬКА ===" << std::endl;
    std::cout << "Владелец: " << ownerName << std::endl;
    std::cout << "Баланс: $" << balance << std::endl;
    std::cout << "Лимиты: $" << minBalance << " - $" << maxBalance << std::endl;
    std::cout << "=====================" << std::endl;
}

void Wallet::displayTransactionHistory() const {
    std::cout << "\n=== TRANSACTION HISTORY ===" << std::endl;
    for (const auto& transaction : transactionHistory) {
        std::cout << getTransactionTypeString(transaction.type) << ": $" << transaction.amount;
        if (!transaction.description.empty()) {
            std::cout << " - " << transaction.description;
        }
        std::cout << " (Balance: $" << transaction.balanceAfter << ")";
        std::cout << " - " << getTimestampString(transaction.timestamp) << std::endl;
    }
    std::cout << "===========================" << std::endl;
}

void Wallet::displayRecentTransactions(int count) const {
    std::cout << "\n=== RECENT TRANSACTIONS ===" << std::endl;
    auto recent = getRecentTransactions(count);
    for (const auto& transaction : recent) {
        std::cout << getTransactionTypeString(transaction.type) << ": $" << transaction.amount;
        if (!transaction.description.empty()) {
            std::cout << " - " << transaction.description;
        }
        std::cout << " (Balance: $" << transaction.balanceAfter << ")";
        std::cout << " - " << getTimestampString(transaction.timestamp) << std::endl;
    }
    std::cout << "===========================" << std::endl;
}

void Wallet::displayStatistics() const {
    std::cout << "\n=== СТАТИСТИКА КОШЕЛЬКА ===" << std::endl;
    std::cout << "Владелец: " << ownerName << std::endl;
    std::cout << "Текущий баланс: $" << balance << std::endl;
    std::cout << "Всего пополнений: $" << getTotalDeposits() << std::endl;
    std::cout << "Всего снятий: $" << getTotalWithdrawals() << std::endl;
    std::cout << "Всего выигрышей: $" << getTotalWinnings() << std::endl;
    std::cout << "Всего проигрышей: $" << getTotalLosses() << std::endl;
    std::cout << "Чистая прибыль: $" << getNetProfit() << std::endl;
    std::cout << "Процент побед: " << std::fixed << std::setprecision(1) << getWinRate() << "%" << std::endl;
    std::cout << "Всего транзакций: " << transactionHistory.size() << std::endl;
    std::cout << "=========================" << std::endl;
}

void Wallet::displaySummary() const {
    std::cout << "Кошелек " << ownerName << ": $" << balance << std::endl;
}

void Wallet::exportToCSV(const std::string& filename) const {
    saveToFile(filename);
}

void Wallet::exportTransactionsToCSV(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cout << "Ошибка: Не удалось открыть файл " << filename << " для записи" << std::endl;
        return;
    }
    
    outFile << "Type,Amount,Description,Timestamp,BalanceAfter" << std::endl;
    for (const auto& transaction : transactionHistory) {
        outFile << getTransactionTypeString(transaction.type) << ","
                << transaction.amount << ","
                << transaction.description << ","
                << getTimestampString(transaction.timestamp) << ","
                << transaction.balanceAfter << std::endl;
    }
    
    outFile.close();
}

void Wallet::clearHistory() {
    transactionHistory.clear();
}

size_t Wallet::getTransactionCount() const {
    return transactionHistory.size();
}

bool Wallet::isEmpty() const {
    return transactionHistory.empty();
}

void Wallet::reset() {
    balance = 1000;
    transactionHistory.clear();
    addTransaction(TransactionType::DEPOSIT, 1000, "Reset to initial balance");
}

void Wallet::setOwner(const std::string& name) {
    ownerName = name;
}

std::string Wallet::getOwner() const {
    return ownerName;
}

void Wallet::addTransaction(TransactionType type, int amount, const std::string& description) {
    Transaction transaction(type, amount, description, balance);
    transactionHistory.push_back(transaction);
}

std::string Wallet::getTransactionTypeString(TransactionType type) const {
    switch (type) {
        case TransactionType::DEPOSIT: return "DEPOSIT";
        case TransactionType::WITHDRAWAL: return "WITHDRAWAL";
        case TransactionType::BET: return "BET";
        case TransactionType::WIN: return "WIN";
        case TransactionType::LOSS: return "LOSS";
        case TransactionType::REFUND: return "REFUND";
        case TransactionType::BONUS: return "BONUS";
        default: return "UNKNOWN";
    }
}

std::string Wallet::getTimestampString(const std::chrono::system_clock::time_point& time) const {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
