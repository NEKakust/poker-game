#include "Result.h"
#include <iostream>

Result::Result() {
    result = GameResult::IN_PROGRESS;
    playerHandName = "";
    dealerHandName = "";
    playerHandRank = 0;
    dealerHandRank = 0;
    potAmount = 0;
    playerFinalHand.clear();
    dealerFinalHand.clear();
    communityCards.clear();
}

void Result::setResult(GameResult gameResult) {
    result = gameResult;
}

void Result::setPlayerHand(const std::string& handName, int rank, const std::vector<Card>& hand) {
    playerHandName = handName;
    playerHandRank = rank;
    playerFinalHand = hand;
}

void Result::setDealerHand(const std::string& handName, int rank, const std::vector<Card>& hand) {
    dealerHandName = handName;
    dealerHandRank = rank;
    dealerFinalHand = hand;
}

void Result::setPotAmount(int amount) {
    potAmount = amount;
}

void Result::setCommunityCards(const std::vector<Card>& cards) {
    communityCards = cards;
}

GameResult Result::getResult() const {
    return result;
}

std::string Result::getPlayerHandName() const {
    return playerHandName;
}

std::string Result::getDealerHandName() const {
    return dealerHandName;
}

int Result::getPlayerHandRank() const {
    return playerHandRank;
}

int Result::getDealerHandRank() const {
    return dealerHandRank;
}

int Result::getPotAmount() const {
    return potAmount;
}

std::vector<Card> Result::getPlayerFinalHand() const {
    return playerFinalHand;
}

std::vector<Card> Result::getDealerFinalHand() const {
    return dealerFinalHand;
}

std::vector<Card> Result::getCommunityCards() const {
    return communityCards;
}

std::string Result::getResultString() const {
    switch (result) {
        case GameResult::PLAYER_WIN:
            return "Player Wins!";
        case GameResult::DEALER_WIN:
            return "Dealer Wins!";
        case GameResult::TIE:
            return "It's a Tie!";
        case GameResult::IN_PROGRESS:
            return "Game in Progress";
        default:
            return "Unknown Result";
    }
}

void Result::displayResult() const {
    std::cout << "\n=== GAME RESULT ===" << std::endl;
    std::cout << "Player Hand: " << playerHandName << " (Rank: " << playerHandRank << ")" << std::endl;
    std::cout << "Dealer Hand: " << dealerHandName << " (Rank: " << dealerHandRank << ")" << std::endl;
    std::cout << "Pot Amount: " << potAmount << std::endl;
    std::cout << "Result: " << getResultString() << std::endl;
    std::cout << "===================" << std::endl;
}

void Result::reset() {
    result = GameResult::IN_PROGRESS;
    playerHandName = "";
    dealerHandName = "";
    playerHandRank = 0;
    dealerHandRank = 0;
    potAmount = 0;
    playerFinalHand.clear();
    dealerFinalHand.clear();
    communityCards.clear();
}
