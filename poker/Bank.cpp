#include "Bank.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

Bank::Bank() {
    potAmount = 0;
    sidePotAmount = 0;
    currentRound = 0;
    maxBet = 0;
    betHistory.clear();
    playerBets.clear();
    playerSidePots.clear();
    activePlayers.clear();
}

void Bank::addToPot(int amount) {
    potAmount += amount;
}

void Bank::addToSidePot(int amount) {
    sidePotAmount += amount;
}

void Bank::setPotAmount(int amount) {
    potAmount = amount;
}

int Bank::getPotAmount() const {
    return potAmount;
}

int Bank::getSidePotAmount() const {
    return sidePotAmount;
}

void Bank::resetPot() {
    potAmount = 0;
}

void Bank::resetSidePot() {
    sidePotAmount = 0;
}

void Bank::placeBet(const std::string& playerName, int amount, bool isAllIn) {
    BetInfo bet;
    bet.amount = amount;
    bet.round = currentRound;
    bet.playerName = playerName;
    bet.isAllIn = isAllIn;
    bet.isFold = false;
    
    betHistory.push_back(bet);
    playerBets[playerName] = amount;
    
    if (amount > maxBet) {
        maxBet = amount;
    }
    
    addToPot(amount);
}

void Bank::foldPlayer(const std::string& playerName) {
    BetInfo bet;
    bet.amount = 0;
    bet.round = currentRound;
    bet.playerName = playerName;
    bet.isAllIn = false;
    bet.isFold = true;
    
    betHistory.push_back(bet);
    
    // Remove from active players
    auto it = std::find(activePlayers.begin(), activePlayers.end(), playerName);
    if (it != activePlayers.end()) {
        activePlayers.erase(it);
    }
}

void Bank::callBet(const std::string& playerName, int callAmount) {
    placeBet(playerName, callAmount);
}

void Bank::raiseBet(const std::string& playerName, int raiseAmount) {
    placeBet(playerName, raiseAmount);
}

int Bank::getPlayerBet(const std::string& playerName) const {
    auto it = playerBets.find(playerName);
    if (it != playerBets.end()) {
        return it->second;
    }
    return 0;
}

int Bank::getMaxBet() const {
    return maxBet;
}

void Bank::setMaxBet(int bet) {
    maxBet = bet;
}

void Bank::startNewRound() {
    currentRound++;
    maxBet = 0;
    playerBets.clear();
}

void Bank::endRound() {
    // Reset bets for next round
    playerBets.clear();
    maxBet = 0;
}

int Bank::getCurrentRound() const {
    return currentRound;
}

void Bank::addPlayer(const std::string& playerName) {
    activePlayers.push_back(playerName);
    playerBets[playerName] = 0;
}

void Bank::removePlayer(const std::string& playerName) {
    auto it = std::find(activePlayers.begin(), activePlayers.end(), playerName);
    if (it != activePlayers.end()) {
        activePlayers.erase(it);
    }
    playerBets.erase(playerName);
}

void Bank::setActivePlayers(const std::vector<std::string>& players) {
    activePlayers = players;
    playerBets.clear();
    for (const auto& player : players) {
        playerBets[player] = 0;
    }
}

std::vector<std::string> Bank::getActivePlayers() const {
    return activePlayers;
}

bool Bank::isPlayerActive(const std::string& playerName) const {
    return std::find(activePlayers.begin(), activePlayers.end(), playerName) != activePlayers.end();
}

void Bank::distributeWinnings(const std::vector<std::string>& winners, const std::vector<int>& amounts) {
    if (winners.size() != amounts.size()) {
        std::cout << "Ошибка: Количество победителей не соответствует количеству сумм" << std::endl;
        return;
    }
    
    std::cout << "\n=== РАСПРЕДЕЛЕНИЕ ВЫИГРЫШЕЙ ===" << std::endl;
    for (size_t i = 0; i < winners.size(); i++) {
        std::cout << winners[i] << " выигрывает: $" << amounts[i] << std::endl;
    }
    std::cout << "=============================" << std::endl;
    
    resetPot();
}

void Bank::distributeSidePot(const std::vector<std::string>& winners, const std::vector<int>& amounts) {
    if (winners.size() != amounts.size()) {
        std::cout << "Ошибка: Количество победителей не соответствует количеству сумм" << std::endl;
        return;
    }
    
    std::cout << "\n=== РАСПРЕДЕЛЕНИЕ САЙД-ПОТА ===" << std::endl;
    for (size_t i = 0; i < winners.size(); i++) {
        std::cout << winners[i] << " выигрывает сайд-пот: $" << amounts[i] << std::endl;
    }
    std::cout << "=============================" << std::endl;
    
    resetSidePot();
}

std::vector<BetInfo> Bank::getBetHistory() const {
    return betHistory;
}

std::vector<BetInfo> Bank::getPlayerBetHistory(const std::string& playerName) const {
    std::vector<BetInfo> playerHistory;
    for (const auto& bet : betHistory) {
        if (bet.playerName == playerName) {
            playerHistory.push_back(bet);
        }
    }
    return playerHistory;
}

int Bank::getTotalBetsByPlayer(const std::string& playerName) const {
    int total = 0;
    for (const auto& bet : betHistory) {
        if (bet.playerName == playerName && !bet.isFold) {
            total += bet.amount;
        }
    }
    return total;
}

int Bank::getRoundBetsByPlayer(const std::string& playerName, int round) const {
    int total = 0;
    for (const auto& bet : betHistory) {
        if (bet.playerName == playerName && bet.round == round && !bet.isFold) {
            total += bet.amount;
        }
    }
    return total;
}

void Bank::displayPot() const {
    std::cout << "Основной банк: $" << potAmount << std::endl;
    if (sidePotAmount > 0) {
        std::cout << "Сайд-пот: $" << sidePotAmount << std::endl;
    }
}

void Bank::displayBetHistory() const {
    std::cout << "\n=== ИСТОРИЯ СТАВОК ===" << std::endl;
    for (const auto& bet : betHistory) {
        std::cout << "Раунд " << bet.round << ": " << bet.playerName;
        if (bet.isFold) {
            std::cout << " сбросил карты";
        } else {
            std::cout << " поставил $" << bet.amount;
            if (bet.isAllIn) {
                std::cout << " (ВА-БАНК)";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "==================" << std::endl;
}

void Bank::displayPlayerBets() const {
    std::cout << "\n=== ТЕКУЩИЕ СТАВКИ ===" << std::endl;
    for (const auto& player : activePlayers) {
        auto it = playerBets.find(player);
        if (it != playerBets.end()) {
            std::cout << player << ": $" << it->second << std::endl;
        }
    }
    std::cout << "===================" << std::endl;
}

bool Bank::allPlayersCalled() const {
    if (activePlayers.empty()) return true;
    
    for (const auto& player : activePlayers) {
        auto it = playerBets.find(player);
        if (it == playerBets.end() || it->second < maxBet) {
            return false;
        }
    }
    return true;
}

void Bank::resetAllBets() {
    playerBets.clear();
    maxBet = 0;
}

void Bank::createSidePot(const std::string& allInPlayer, int allInAmount) {
    // Move excess bets to side pot
    for (auto& player : playerBets) {
        if (player.first != allInPlayer && player.second > allInAmount) {
            int excess = player.second - allInAmount;
            addToSidePot(excess);
            player.second = allInAmount;
        }
    }
}

std::map<std::string, int> Bank::getSidePotDistribution() const {
    return playerSidePots;
}
