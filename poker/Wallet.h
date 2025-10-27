#ifndef POKER_WALLET_H
#define POKER_WALLET_H

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <map>

enum class TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    BET,
    WIN,
    LOSS,
    REFUND,
    BONUS
};

struct Transaction {
    TransactionType type;
    int amount;
    std::chrono::system_clock::time_point timestamp;
    std::string description;
    int balanceAfter;
    
    Transaction(TransactionType t, int amt, const std::string& desc, int balance)
        : type(t), amount(amt), timestamp(std::chrono::system_clock::now()), 
          description(desc), balanceAfter(balance) {}
};

class Wallet {
private:
    int balance;
    std::string ownerName;
    std::vector<Transaction> transactionHistory;
    std::string filename;
    bool autoSave;
    int maxBalance;
    int minBalance;
    
    std::string getTransactionTypeString(TransactionType type) const;
    std::string getTimestampString(const std::chrono::system_clock::time_point& time) const;
    void addTransaction(TransactionType type, int amount, const std::string& description);

public:
    Wallet();
    Wallet(const std::string& owner, int initialBalance = 1000);
    Wallet(const std::string& owner, int initialBalance, const std::string& file);
    
    bool deposit(int amount, const std::string& description = "Deposit");
    bool withdraw(int amount, const std::string& description = "Withdrawal");
    bool canAfford(int amount) const;
    int getBalance() const;
    void setBalance(int amount);
    
    bool placeBet(int amount, const std::string& gameId = "");
    void winBet(int amount, const std::string& gameId = "");
    void loseBet(int amount, const std::string& gameId = "");
    void refundBet(int amount, const std::string& gameId = "");
    
    void addBonus(int amount, const std::string& reason = "Bonus");
    
    void setMaxBalance(int max);
    void setMinBalance(int min);
    int getMaxBalance() const;
    int getMinBalance() const;
    bool isWithinLimits(int amount) const;
    
    std::vector<Transaction> getTransactionHistory() const;
    std::vector<Transaction> getTransactionsByType(TransactionType type) const;
    std::vector<Transaction> getRecentTransactions(int count = 10) const;
    int getTotalDeposits() const;
    int getTotalWithdrawals() const;
    int getTotalWinnings() const;
    int getTotalLosses() const;
    
    double getWinRate() const;
    int getNetProfit() const;
    std::map<std::string, int> getDailyStats() const;
    std::map<std::string, int> getMonthlyStats() const;
    
    void saveToFile() const;
    void saveToFile(const std::string& file) const;
    void loadFromFile();
    void loadFromFile(const std::string& file);
    void setAutoSave(bool enable);
    void setFilename(const std::string& file);
    
    void displayBalance() const;
    void displayTransactionHistory() const;
    void displayRecentTransactions(int count = 10) const;
    void displayStatistics() const;
    void displaySummary() const;
    
    void exportToCSV(const std::string& filename) const;
    void exportTransactionsToCSV(const std::string& filename) const;
    
    void clearHistory();
    size_t getTransactionCount() const;
    bool isEmpty() const;
    void reset();
    
    void setOwner(const std::string& name);
    std::string getOwner() const;
};

#endif
