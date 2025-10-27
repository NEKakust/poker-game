#ifndef POKER_BOTPLAYER_H
#define POKER_BOTPLAYER_H

#include "Player.h"
#include <vector>
#include <string>
#include <random>

enum class BotDifficulty {
    EASY,
    MEDIUM,
    HARD
};

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
    BotDifficulty difficulty;
    int bankroll;
    int currentBet;
    double aggressionFactor;
    double bluffingFactor;
    double tightnessFactor;
    std::mt19937 rng;
    
    BotDecision makeDecision(const std::vector<Card>& communityCards, 
                             int potAmount, int currentBet, int maxBet);
    
    double evaluateHandStrength(const std::vector<Card>& hand, 
                               const std::vector<Card>& communityCards);
    double calculateWinProbability(const std::vector<Card>& hand, 
                                  const std::vector<Card>& communityCards);
    
    bool shouldBluff(double handStrength, int potOdds);
    bool shouldCall(double handStrength, int potOdds, int betAmount);
    bool shouldRaise(double handStrength, int potOdds);
    int calculateRaiseAmount(double handStrength, int potAmount);
    
    int getRandomAmount(int min, int max);
    double getRandomDouble(double min, double max);
    std::string getActionString(BotAction action) const;
    
public:
    BotPlayer(const std::string& name, BotDifficulty diff = BotDifficulty::MEDIUM);
    BotPlayer(const std::string& name, int initialBankroll, BotDifficulty diff = BotDifficulty::MEDIUM);
    
    void setBankroll(int amount);
    int getBankroll() const;
    void setCurrentBet(int bet);
    int getCurrentBet() const;
    
    BotDecision getAction(const std::vector<Card>& communityCards, 
                         int potAmount, int currentBet, int maxBet);
    void adjustPersonality(double aggression, double bluffing, double tightness);
    
    void setDifficulty(BotDifficulty diff);
    BotDifficulty getDifficulty() const;
    
    void displayPersonality() const;
    void displayDecision(const BotDecision& decision) const;
    
    bool canAffordBet(int amount) const;
    int getMaxBet() const;
    void resetForNewHand();
};

#endif
