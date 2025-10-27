#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include "Card.h"

enum class GameResult {
    PLAYER_WIN,
    DEALER_WIN,
    TIE,
    IN_PROGRESS
};

class Result {
private:
    GameResult result;
    std::string playerHandName;
    std::string dealerHandName;
    int playerHandRank;
    int dealerHandRank;
    int potAmount;
    std::vector<Card> playerFinalHand;
    std::vector<Card> dealerFinalHand;
    std::vector<Card> communityCards;

public:
    Result();
    void setResult(GameResult gameResult);
    void setPlayerHand(const std::string& handName, int rank, const std::vector<Card>& hand);
    void setDealerHand(const std::string& handName, int rank, const std::vector<Card>& hand);
    void setPotAmount(int amount);
    void setCommunityCards(const std::vector<Card>& cards);

    GameResult getResult() const;
    std::string getPlayerHandName() const;
    std::string getDealerHandName() const;
    int getPlayerHandRank() const;
    int getDealerHandRank() const;
    int getPotAmount() const;
    std::vector<Card> getPlayerFinalHand() const;
    std::vector<Card> getDealerFinalHand() const;
    std::vector<Card> getCommunityCards() const;

    std::string getResultString() const;
    void displayResult() const;
    void reset();
};

#endif

