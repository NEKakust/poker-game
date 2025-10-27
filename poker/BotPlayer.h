#ifndef POKER_BOTPLAYER_H
#define POKER_BOTPLAYER_H

#include "Player.h"
#include <vector>
#include <string>
#include <random>

enum class BotAction {
    FOLD,
    CHECK,
    CALL,
    RAISE,
    ALL_IN
};

struct BotDecision {
    BotAction action;
    int amount;
    std::string reasoning;
};

class BotPlayer : public Player {
private:
    int bankroll;
    int currentBet;
    std::mt19937 rng;
    
    BotDecision makeDecision(const std::vector<Card>& communityCards, 
                             int potAmount, int currentBet, int maxBet);
    double evaluateHandStrength(const std::vector<Card>& hand, 
                               const std::vector<Card>& communityCards);
    int calculateRaiseAmount(double handStrength, int potAmount);
    int getRandomAmount(int min, int max);
    double getRandomDouble(double min, double max);
    std::string getActionString(BotAction action) const;
    
public:
    BotPlayer(const std::string& name);
    BotPlayer(const std::string& name, int initialBankroll);
    
    void setBankroll(int amount);
    int getBankroll() const;
    void setCurrentBet(int bet);
    int getCurrentBet() const;
    BotDecision getAction(const std::vector<Card>& communityCards, 
                         int potAmount, int currentBet, int maxBet);
    void displayDecision(const BotDecision& decision) const;
    bool canAffordBet(int amount) const;
    int getMaxBet() const;
    void resetForNewHand();
};

#endif
