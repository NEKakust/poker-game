#ifndef POKER_BANK_H
#define POKER_BANK_H

#include <vector>
#include <map>
#include "Player.h"

struct BetInfo {
    int amount;
    int round;
    std::string playerName;
    bool isAllIn;
    bool isFold;
};

class Bank {
private:
    int potAmount;
    int sidePotAmount;
    std::vector<BetInfo> betHistory;
    std::map<std::string, int> playerBets;
    std::map<std::string, int> playerSidePots;
    int currentRound;
    int maxBet;
    std::vector<std::string> activePlayers;

public:
    Bank();
    
    // Pot management
    void addToPot(int amount);
    void addToSidePot(int amount);
    void setPotAmount(int amount);
    int getPotAmount() const;
    int getSidePotAmount() const;
    void resetPot();
    void resetSidePot();
    
    // Betting management
    void placeBet(const std::string& playerName, int amount, bool isAllIn = false);
    void foldPlayer(const std::string& playerName);
    void callBet(const std::string& playerName, int callAmount);
    void raiseBet(const std::string& playerName, int raiseAmount);
    int getPlayerBet(const std::string& playerName) const;
    int getMaxBet() const;
    void setMaxBet(int bet);
    
    // Round management
    void startNewRound();
    void endRound();
    int getCurrentRound() const;
    
    // Player management
    void addPlayer(const std::string& playerName);
    void removePlayer(const std::string& playerName);
    void setActivePlayers(const std::vector<std::string>& players);
    std::vector<std::string> getActivePlayers() const;
    bool isPlayerActive(const std::string& playerName) const;
    
    // Distribution
    void distributeWinnings(const std::vector<std::string>& winners, const std::vector<int>& amounts);
    void distributeSidePot(const std::vector<std::string>& winners, const std::vector<int>& amounts);
    
    // History and statistics
    std::vector<BetInfo> getBetHistory() const;
    std::vector<BetInfo> getPlayerBetHistory(const std::string& playerName) const;
    int getTotalBetsByPlayer(const std::string& playerName) const;
    int getRoundBetsByPlayer(const std::string& playerName, int round) const;
    
    // Utility methods
    void displayPot() const;
    void displayBetHistory() const;
    void displayPlayerBets() const;
    bool allPlayersCalled() const;
    void resetAllBets();
    
    // Side pot management for all-in scenarios
    void createSidePot(const std::string& allInPlayer, int allInAmount);
    std::map<std::string, int> getSidePotDistribution() const;
};

#endif //POKER_BANK_H
