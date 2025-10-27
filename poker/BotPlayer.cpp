#include "BotPlayer.h"
#include "HandEvaluator.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

BotPlayer::BotPlayer(const std::string& name, BotDifficulty diff) 
    : Player(name), difficulty(diff), bankroll(1000), currentBet(0) {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    
    // Set personality based on difficulty
    switch (difficulty) {
        case BotDifficulty::EASY:
            aggressionFactor = 0.3;
            bluffingFactor = 0.2;
            tightnessFactor = 0.7;
            break;
        case BotDifficulty::MEDIUM:
            aggressionFactor = 0.5;
            bluffingFactor = 0.4;
            tightnessFactor = 0.5;
            break;
        case BotDifficulty::HARD:
            aggressionFactor = 0.7;
            bluffingFactor = 0.6;
            tightnessFactor = 0.3;
            break;
    }
}

BotPlayer::BotPlayer(const std::string& name, int initialBankroll, BotDifficulty diff) 
    : Player(name), difficulty(diff), bankroll(initialBankroll), currentBet(0) {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    
    // Set personality based on difficulty
    switch (difficulty) {
        case BotDifficulty::EASY:
            aggressionFactor = 0.3;
            bluffingFactor = 0.2;
            tightnessFactor = 0.7;
            break;
        case BotDifficulty::MEDIUM:
            aggressionFactor = 0.5;
            bluffingFactor = 0.4;
            tightnessFactor = 0.5;
            break;
        case BotDifficulty::HARD:
            aggressionFactor = 0.7;
            bluffingFactor = 0.6;
            tightnessFactor = 0.3;
            break;
    }
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
    
    // Get current hand strength
    std::vector<Card> fullHand = getHand();
    fullHand.insert(fullHand.end(), communityCards.begin(), communityCards.end());
    
    double handStrength = evaluateHandStrength(getHand(), communityCards);
    double winProbability = calculateWinProbability(getHand(), communityCards);
    
    // Calculate pot odds
    int potOdds = (potAmount > 0) ? (currentBet * 100) / potAmount : 0;
    
    // Make decision based on difficulty and personality
    switch (difficulty) {
        case BotDifficulty::EASY:
            // Очень агрессивная и рискованная игра
            if (handStrength > 0.5) {
                // Рейз даже при средних руках
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "Рискну с такой рукой, рейз!";
            } else if (handStrength > 0.25) {
                // Колл при слабых руках
                decision.action = BotAction::CALL;
                decision.amount = currentBet;
                decision.reasoning = "Попробую рискнуть с такой рукой";
            } else if (handStrength > 0.1) {
                // Блеф при очень слабых руках
                if (shouldBluff(handStrength, potOdds)) {
                    decision.action = BotAction::RAISE;
                    decision.amount = calculateRaiseAmount(handStrength, potAmount);
                    decision.reasoning = "Блеф! Риск-бизнес!";
                } else {
                    decision.action = BotAction::CALL;
                    decision.amount = currentBet;
                    decision.reasoning = "Рискую посмотреть следующие карты";
                }
            } else {
                // Сброс только при совсем слабых руках
                decision.action = BotAction::FOLD;
                decision.amount = 0;
                decision.reasoning = "Слишком слабая рука";
            }
            break;
            
        case BotDifficulty::MEDIUM:
            // More sophisticated decision making
            if (handStrength > 0.8) {
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "Очень сильная рука, рейз";
            } else if (handStrength > 0.6) {
                if (shouldBluff(handStrength, potOdds)) {
                    decision.action = BotAction::RAISE;
                    decision.amount = calculateRaiseAmount(handStrength, potAmount);
                    decision.reasoning = "Хорошая рука, рейз";
                } else {
                    decision.action = BotAction::CALL;
                    decision.amount = currentBet;
                    decision.reasoning = "Хорошая рука, колл";
                }
            } else if (handStrength > 0.3) {
                if (shouldCall(handStrength, potOdds, currentBet)) {
                    decision.action = BotAction::CALL;
                    decision.amount = currentBet;
                    decision.reasoning = "Пограничная рука, колл";
                } else {
                    decision.action = BotAction::FOLD;
                    decision.amount = 0;
                    decision.reasoning = "Пограничная рука, сброс";
                }
            } else {
                decision.action = BotAction::FOLD;
                decision.amount = 0;
                decision.reasoning = "Слабая рука, сброс";
            }
            break;
            
        case BotDifficulty::HARD:
            // Advanced decision making with bluffing
            if (handStrength > 0.9) {
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "Отличная рука, рейз";
            } else if (handStrength > 0.7) {
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "Сильная рука, рейз";
            } else if (handStrength > 0.5) {
                if (shouldBluff(handStrength, potOdds)) {
                    decision.action = BotAction::RAISE;
                    decision.amount = calculateRaiseAmount(handStrength, potAmount);
                    decision.reasoning = "Блеф с неплохой рукой";
                } else {
                    decision.action = BotAction::CALL;
                    decision.amount = currentBet;
                    decision.reasoning = "Неплохая рука, колл";
                }
            } else if (handStrength > 0.2) {
                if (shouldBluff(handStrength, potOdds)) {
                    decision.action = BotAction::CALL;
                    decision.amount = currentBet;
                    decision.reasoning = "Блеф со слабой рукой";
                } else {
                    decision.action = BotAction::FOLD;
                    decision.amount = 0;
                    decision.reasoning = "Слабая рука, сброс";
                }
            } else {
                decision.action = BotAction::FOLD;
                decision.amount = 0;
                decision.reasoning = "Очень слабая рука, сброс";
            }
            break;
    }
    
    // Ensure bot can afford the bet
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
    
    // Combine hand and community cards
    std::vector<Card> fullHand = hand;
    fullHand.insert(fullHand.end(), communityCards.begin(), communityCards.end());
    
    // Use HandEvaluator to get hand rank
    HandEvaluation evaluation = HandEvaluator::evaluateHand(fullHand);
    
    // Convert rank to strength (0.0 to 1.0)
    double strength = static_cast<double>(evaluation.rankValue) / 9.0;
    
    // Adjust based on kickers for same-rank hands
    if (!evaluation.kickers.empty()) {
        double kickerBonus = 0.0;
        for (int kicker : evaluation.kickers) {
            kickerBonus += static_cast<double>(kicker) / 14.0 * 0.1;
        }
        strength += kickerBonus;
    }
    
    return std::min(1.0, strength);
}

double BotPlayer::calculateWinProbability(const std::vector<Card>& hand, 
                                         const std::vector<Card>& communityCards) {
    // Simplified win probability calculation
    double handStrength = evaluateHandStrength(hand, communityCards);
    
    // Adjust based on number of community cards
    double communityFactor = 1.0;
    if (communityCards.size() >= 3) {
        communityFactor = 0.8; // More cards revealed, less uncertainty
    }
    
    return handStrength * communityFactor;
}

bool BotPlayer::shouldBluff(double handStrength, int potOdds) {
    double bluffChance = bluffingFactor * (1.0 - handStrength);
    double randomFactor = getRandomDouble(0.0, 1.0);
    
    return randomFactor < bluffChance;
}

bool BotPlayer::shouldCall(double handStrength, int potOdds, int betAmount) {
    double callThreshold = tightnessFactor * 0.5;
    return handStrength > callThreshold;
}

bool BotPlayer::shouldRaise(double handStrength, int potOdds) {
    double raiseThreshold = tightnessFactor * 0.6;
    return handStrength > raiseThreshold;
}

int BotPlayer::calculateRaiseAmount(double handStrength, int potAmount) {
    int baseRaise = potAmount / 2;
    double aggressionMultiplier = 1.0 + aggressionFactor;
    
    int raiseAmount = static_cast<int>(baseRaise * aggressionMultiplier * handStrength);
    
    // Add some randomness
    int randomFactor = getRandomAmount(-10, 20);
    raiseAmount += randomFactor;
    
    return std::max(10, raiseAmount); // Minimum raise of 10
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

void BotPlayer::adjustPersonality(double aggression, double bluffing, double tightness) {
    aggressionFactor = std::max(0.0, std::min(1.0, aggression));
    bluffingFactor = std::max(0.0, std::min(1.0, bluffing));
    tightnessFactor = std::max(0.0, std::min(1.0, tightness));
}

void BotPlayer::setDifficulty(BotDifficulty diff) {
    difficulty = diff;
    
    // Adjust personality based on new difficulty
    switch (difficulty) {
        case BotDifficulty::EASY:
            aggressionFactor = 0.3;
            bluffingFactor = 0.2;
            tightnessFactor = 0.7;
            break;
        case BotDifficulty::MEDIUM:
            aggressionFactor = 0.5;
            bluffingFactor = 0.4;
            tightnessFactor = 0.5;
            break;
        case BotDifficulty::HARD:
            aggressionFactor = 0.7;
            bluffingFactor = 0.6;
            tightnessFactor = 0.3;
            break;
    }
}

BotDifficulty BotPlayer::getDifficulty() const {
    return difficulty;
}

void BotPlayer::displayPersonality() const {
    std::cout << "\n=== ЛИЧНОСТЬ " << getName() << " ===" << std::endl;
    std::cout << "Сложность: ";
    switch (difficulty) {
        case BotDifficulty::EASY: std::cout << "ЛЕГКАЯ"; break;
        case BotDifficulty::MEDIUM: std::cout << "СРЕДНЯЯ"; break;
        case BotDifficulty::HARD: std::cout << "СЛОЖНАЯ"; break;
    }
    std::cout << std::endl;
    std::cout << "Агрессивность: " << std::fixed << std::setprecision(2) << aggressionFactor << std::endl;
    std::cout << "Блеф: " << std::fixed << std::setprecision(2) << bluffingFactor << std::endl;
    std::cout << "Осторожность: " << std::fixed << std::setprecision(2) << tightnessFactor << std::endl;
    std::cout << "Банкролл: $" << bankroll << std::endl;
    std::cout << "=========================" << std::endl;
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
