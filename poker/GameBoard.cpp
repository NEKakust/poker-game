#include "GameBoard.h"
#include <iostream>
#include <iomanip>

GameBoard::GameBoard() {
    currentPhase = GamePhase::PREFLOP;
    potAmount = 0;
    currentBet = 0;
    smallBlind = 5;
    bigBlind = 10;
    dealerPosition = 0;
    currentPlayerIndex = 0;
    communityCards.clear();
    players.clear();
}

GameBoard::GameBoard(int smallBlindAmount, int bigBlindAmount) {
    currentPhase = GamePhase::PREFLOP;
    potAmount = 0;
    currentBet = 0;
    smallBlind = smallBlindAmount;
    bigBlind = bigBlindAmount;
    dealerPosition = 0;
    currentPlayerIndex = 0;
    communityCards.clear();
    players.clear();
}

void GameBoard::setPhase(GamePhase phase) {
    currentPhase = phase;
}

GamePhase GameBoard::getCurrentPhase() const {
    return currentPhase;
}

void GameBoard::advancePhase() {
    switch (currentPhase) {
        case GamePhase::PREFLOP:
            currentPhase = GamePhase::FLOP;
            break;
        case GamePhase::FLOP:
            currentPhase = GamePhase::TURN;
            break;
        case GamePhase::TURN:
            currentPhase = GamePhase::RIVER;
            break;
        case GamePhase::RIVER:
            currentPhase = GamePhase::SHOWDOWN;
            break;
        case GamePhase::SHOWDOWN:
            // Game complete, don't advance
            break;
    }
}

std::string GameBoard::getPhaseString() const {
    switch (currentPhase) {
        case GamePhase::PREFLOP:
            return "Pre-flop";
        case GamePhase::FLOP:
            return "Flop";
        case GamePhase::TURN:
            return "Turn";
        case GamePhase::RIVER:
            return "River";
        case GamePhase::SHOWDOWN:
            return "Showdown";
        default:
            return "Unknown";
    }
}

void GameBoard::addCommunityCard(const Card& card) {
    communityCards.push_back(card);
}

void GameBoard::addFlopCards(const std::vector<Card>& cards) {
    if (cards.size() >= 3) {
        for (int i = 0; i < 3; i++) {
            communityCards.push_back(cards[i]);
        }
    }
}

void GameBoard::addTurnCard(const Card& card) {
    if (communityCards.size() == 3) {
        communityCards.push_back(card);
    }
}

void GameBoard::addRiverCard(const Card& card) {
    if (communityCards.size() == 4) {
        communityCards.push_back(card);
    }
}

std::vector<Card> GameBoard::getCommunityCards() const {
    return communityCards;
}

void GameBoard::clearCommunityCards() {
    communityCards.clear();
}

void GameBoard::addToPot(int amount) {
    potAmount += amount;
}

void GameBoard::setPotAmount(int amount) {
    potAmount = amount;
}

int GameBoard::getPotAmount() const {
    return potAmount;
}

void GameBoard::resetPot() {
    potAmount = 0;
}

void GameBoard::setCurrentBet(int bet) {
    currentBet = bet;
}

int GameBoard::getCurrentBet() const {
    return currentBet;
}

void GameBoard::setBlinds(int small, int big) {
    smallBlind = small;
    bigBlind = big;
}

int GameBoard::getSmallBlind() const {
    return smallBlind;
}

int GameBoard::getBigBlind() const {
    return bigBlind;
}

void GameBoard::addPlayer(Player* player) {
    players.push_back(player);
}

void GameBoard::removePlayer(Player* player) {
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (*it == player) {
            players.erase(it);
            break;
        }
    }
}

std::vector<Player*> GameBoard::getPlayers() const {
    return players;
}

Player* GameBoard::getCurrentPlayer() const {
    if (currentPlayerIndex < players.size()) {
        return players[currentPlayerIndex];
    }
    return nullptr;
}

void GameBoard::setCurrentPlayer(int index) {
    if (index >= 0 && index < players.size()) {
        currentPlayerIndex = index;
    }
}

void GameBoard::nextPlayer() {
    currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
}

void GameBoard::setDealerPosition(int position) {
    if (position >= 0 && position < players.size()) {
        dealerPosition = position;
    }
}

int GameBoard::getDealerPosition() const {
    return dealerPosition;
}

void GameBoard::displayBoard() const {
    std::cout << "\n=== GAME BOARD ===" << std::endl;
    std::cout << "Phase: " << getPhaseString() << std::endl;
    std::cout << "Pot: $" << potAmount << std::endl;
    std::cout << "Current Bet: $" << currentBet << std::endl;
    std::cout << "Blinds: $" << smallBlind << "/$" << bigBlind << std::endl;
    
    if (!communityCards.empty()) {
        std::cout << "\nCommunity Cards:" << std::endl;
        for (size_t i = 0; i < communityCards.size(); i++) {
            std::cout << "  " << (i + 1) << ". " << communityCards[i].getRank() 
                      << " of " << communityCards[i].getSuit() << std::endl;
        }
    }
    
    std::cout << "==================" << std::endl;
}

void GameBoard::displayCommunityCards() const {
    if (!communityCards.empty()) {
        std::cout << "\nCommunity Cards: ";
        for (const auto& card : communityCards) {
            std::cout << card.getRank() << card.getSuit() << " ";
        }
        std::cout << std::endl;
    }
}

void GameBoard::displayPot() const {
    std::cout << "Pot: $" << potAmount << std::endl;
}

void GameBoard::resetBoard() {
    currentPhase = GamePhase::PREFLOP;
    potAmount = 0;
    currentBet = 0;
    communityCards.clear();
    currentPlayerIndex = 0;
}

bool GameBoard::isGameComplete() const {
    return currentPhase == GamePhase::SHOWDOWN;
}
