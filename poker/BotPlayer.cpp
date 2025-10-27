#include "BotPlayer.h"
#include "HandEvaluator.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

BotPlayer::BotPlayer(const std::string& name) 
    : Player(name), bankroll(1000), currentBet(0) {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

BotPlayer::BotPlayer(const std::string& name, int initialBankroll) 
    : Player(name), bankroll(initialBankroll), currentBet(0) {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

void BotPlayer::setBankroll(int amount) {
    bankroll = amount;
}

int BotPlayer::getBankroll() const {
    return bankroll;
}

void BotPlayer::setCurrentBet(int bet) {
    currentBet = bet;
}

int BotPlayer::getCurrentBet() const {
    return currentBet;
}

BotDecision BotPlayer::getAction(const std::vector<Card>& communityCards, 
                                int potAmount, int currentBet, int maxBet) {
    return makeDecision(communityCards, potAmount, currentBet, maxBet);
}

BotDecision BotPlayer::makeDecision(const std::vector<Card>& communityCards, 
                                   int potAmount, int currentBet, int maxBet) {
    BotDecision decision;
    
    double handStrength = evaluateHandStrength(getHand(), communityCards);
    double randomRisk = getRandomDouble(0.0, 1.0);
    
    if (handStrength > 0.6) {
        if (randomRisk < 0.7) {
            decision.action = BotAction::ALL_IN;
            decision.amount = bankroll;
            decision.reasoning = "ВА-БАНК! Сильная рука!";
        } else {
            decision.action = BotAction::RAISE;
            decision.amount = calculateRaiseAmount(handStrength, potAmount);
            decision.reasoning = "РЕЙЗ! Иду до победы!";
        }
    } else if (handStrength > 0.1) {
        decision.action = BotAction::RAISE;
        decision.amount = calculateRaiseAmount(handStrength, potAmount);
        decision.reasoning = "РЕЙЗ! Рискую!";
    } else {
        decision.action = BotAction::CALL;
        decision.amount = currentBet;
        decision.reasoning = "КОЛЛ! Никогда не сдаюсь!";
    }
    
    if (decision.amount > bankroll) {
        decision.amount = bankroll;
        decision.action = BotAction::ALL_IN;
        decision.reasoning += " (ВА-БАНК)";
    }
    
    return decision;
}

double BotPlayer::evaluateHandStrength(const std::vector<Card>& hand, 
                                      const std::vector<Card>& communityCards) {
    if (hand.empty()) return 0.0;
    
    std::vector<Card> fullHand = hand;
    fullHand.insert(fullHand.end(), communityCards.begin(), communityCards.end());
    
    HandEvaluation evaluation = HandEvaluator::evaluateHand(fullHand);
    
    double strength = static_cast<double>(evaluation.rankValue) / 9.0;
    
    if (!evaluation.kickers.empty()) {
        double kickerBonus = 0.0;
        for (int kicker : evaluation.kickers) {
            kickerBonus += static_cast<double>(kicker) / 14.0 * 0.1;
        }
        strength += kickerBonus;
    }
    
    return std::min(1.0, strength);
}

int BotPlayer::calculateRaiseAmount(double handStrength, int potAmount) {
    int baseRaise = static_cast<int>(potAmount * 0.75);
    int raiseAmount = static_cast<int>(baseRaise * handStrength);
    
    int randomFactor = getRandomAmount(0, 50);
    raiseAmount += randomFactor;
    
    return std::max(30, raiseAmount);
}

int BotPlayer::getRandomAmount(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

double BotPlayer::getRandomDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

std::string BotPlayer::getActionString(BotAction action) const {
    switch (action) {
        case BotAction::FOLD: return "СБРОС";
        case BotAction::CHECK: return "ПАС";
        case BotAction::CALL: return "КОЛЛ";
        case BotAction::RAISE: return "РЕЙЗ";
        case BotAction::ALL_IN: return "ВА-БАНК";
        default: return "НЕИЗВЕСТНО";
    }
}

void BotPlayer::displayDecision(const BotDecision& decision) const {
    std::cout << getName() << " решает " << getActionString(decision.action);
    if (decision.amount > 0) {
        std::cout << " $" << decision.amount;
    }
    std::cout << " - " << decision.reasoning << std::endl;
}

bool BotPlayer::canAffordBet(int amount) const {
    return amount <= bankroll;
}

int BotPlayer::getMaxBet() const {
    return bankroll;
}

void BotPlayer::resetForNewHand() {
    currentBet = 0;
    clearHand();
}
