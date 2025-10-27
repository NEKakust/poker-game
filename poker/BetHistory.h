#ifndef POKER_BETHISTORY_H
#define POKER_BETHISTORY_H

#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <map>

enum class BetAction {
    FOLD,
    CHECK,
    CALL,
    RAISE,
    ALL_IN,
    SMALL_BLIND,
    BIG_BLIND
};

struct BetRecord {
    std::string playerName;
    BetAction action;
    int amount;
    int round;
    std::chrono::system_clock::time_point timestamp;
    std::string notes;
    
    BetRecord(const std::string& name, BetAction act, int amt, int rnd, const std::string& note = "")
        : playerName(name), action(act), amount(amt), round(rnd), 
          timestamp(std::chrono::system_clock::now()), notes(note) {}
};

class BetHistory {
private:
    std::vector<BetRecord> history;
    std::string filename;
    bool autoSave;
    
    std::string getActionString(BetAction action) const;
    std::string getTimestampString(const std::chrono::system_clock::time_point& time) const;

public:
    BetHistory();
    BetHistory(const std::string& file);
    
    // Record management
    void addBet(const std::string& playerName, BetAction action, int amount, int round, const std::string& notes = "");
    void addFold(const std::string& playerName, int round);
    void addCheck(const std::string& playerName, int round);
    void addCall(const std::string& playerName, int amount, int round);
    void addRaise(const std::string& playerName, int amount, int round);
    void addAllIn(const std::string& playerName, int amount, int round);
    void addBlind(const std::string& playerName, int amount, int round, bool isBigBlind = false);
    
    // History retrieval
    std::vector<BetRecord> getHistory() const;
    std::vector<BetRecord> getPlayerHistory(const std::string& playerName) const;
    std::vector<BetRecord> getRoundHistory(int round) const;
    std::vector<BetRecord> getPlayerRoundHistory(const std::string& playerName, int round) const;
    
    // Statistics
    int getTotalBetsByPlayer(const std::string& playerName) const;
    int getTotalBetsByPlayerInRound(const std::string& playerName, int round) const;
    int getTotalPotInRound(int round) const;
    int getPlayerActionCount(const std::string& playerName, BetAction action) const;
    double getPlayerAggressionFactor(const std::string& playerName) const;
    
    // Analysis
    std::vector<std::string> getMostAggressivePlayers() const;
    std::vector<std::string> getTightestPlayers() const;
    std::map<std::string, int> getPlayerStats(const std::string& playerName) const;
    
    // File operations
    void saveToFile() const;
    void saveToFile(const std::string& file) const;
    void loadFromFile();
    void loadFromFile(const std::string& file);
    void setAutoSave(bool enable);
    void setFilename(const std::string& file);
    
    // Display methods
    void displayHistory() const;
    void displayPlayerHistory(const std::string& playerName) const;
    void displayRoundHistory(int round) const;
    void displayStatistics() const;
    void displayPlayerStatistics(const std::string& playerName) const;
    
    // Utility methods
    void clearHistory();
    void clearPlayerHistory(const std::string& playerName);
    void clearRoundHistory(int round);
    size_t getHistorySize() const;
    bool isEmpty() const;
    
    // Export methods
    void exportToCSV(const std::string& filename) const;
    void exportPlayerToCSV(const std::string& playerName, const std::string& filename) const;
};

#endif //POKER_BETHISTORY_H
