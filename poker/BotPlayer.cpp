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
        case BotDifficulty::EASY: {
            // БОТ НИКОГДА НЕ СБРАСЫВАЕТ - ВСЕГДА ИГРАЕТ ДО КОНЦА
            double randomRisk = getRandomDouble(0.0, 1.0);
            
            if (handStrength > 0.6) {
                // Очень сильные руки - ва-банк с вероятностью 70%
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
                // Средние руки - рейз
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "РЕЙЗ! Рискую!";
            } else {
                // Слабые руки - колл
                decision.action = BotAction::CALL;
                decision.amount = currentBet;
                decision.reasoning = "КОЛЛ! Никогда не сдаюсь!";
            }
            }
            break;
            
        case BotDifficulty::MEDIUM: {
            // NO FOLDING - always play
            if (handStrength > 0.5) {
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "Сильная рука - рейз!";
            } else {
                decision.action = BotAction::CALL;
                decision.amount = currentBet;
                decision.reasoning = "Колл - иду до конца!";
            }
            }
            break;
            
        case BotDifficulty::HARD: {
            // NO FOLDING - always play
            if (handStrength > 0.6) {
                decision.action = BotAction::RAISE;
                decision.amount = calculateRaiseAmount(handStrength, potAmount);
                decision.reasoning = "Сильная рука - рейз!";
            } else {
                decision.action = BotAction::CALL;
                decision.amount = currentBet;
                decision.reasoning = "Колл - все или ничего!";
            }
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
    // ОЧЕНЬ агрессивный блеф - бот почти всегда рискует
    double bluffChance = (bluffingFactor + 0.6) * (1.0 - handStrength); // +0.6 вместо +0.3
    
    // Дополнительно повышаем шанс блефа при больших банках
    if (potOdds > 20) {
        bluffChance += 0.3; // Еще больше риска при крупном банке
    }
    
    // Минимальный шанс блефа 50%
    bluffChance = std::max(0.5, std::min(0.95, bluffChance));
    
    double randomFactor = getRandomDouble(0.0, 1.0);
    
    return randomFactor < bluffChance;
}

bool BotPlayer::shouldCall(double handStrength, int potOdds, int betAmount) {
    // Более агрессивный порог для колла
    double callThreshold = tightnessFactor * 0.3; // Снижаем порог с 0.5 до 0.3
    
    // Добавляем бонус за привлекательный банк
    if (potOdds < 30) {
        callThreshold -= 0.1; // Еще ниже порог при хороших шансах
    }
    
    return handStrength > std::max(0.1, callThreshold); // Минимум 0.1
}

bool BotPlayer::shouldRaise(double handStrength, int potOdds) {
    // Снижаем порог для рейза - бот становится более агрессивным
    double raiseThreshold = tightnessFactor * 0.4; // Снижаем с 0.6 до 0.4
    
    // Добавляем бонус при больших банках
    if (potOdds > 25) {
        raiseThreshold -= 0.1; // Еще ниже порог при крупном банке
    }
    
    return handStrength > std::max(0.2, raiseThreshold); // Минимум 0.2
}

int BotPlayer::calculateRaiseAmount(double handStrength, int potAmount) {
    // МАКСИМАЛЬНО агрессивные ставки - бот ставит очень большие суммы
    int baseRaise = static_cast<int>(potAmount * 1.2); // Увеличиваем с 0.75 до 1.2
    double aggressionMultiplier = 1.0 + aggressionFactor + 1.0; // Добавляем +1.0 к агрессивности
    
    int raiseAmount = static_cast<int>(baseRaise * aggressionMultiplier * handStrength);
    
    // МАКСИМАЛЬНАЯ случайность для полной непредсказуемости
    int randomFactor = getRandomAmount(0, 100); // Увеличиваем с 50 до 100
    raiseAmount += randomFactor;
    
    // Минимальный рейз $30, но часто будет больше
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
