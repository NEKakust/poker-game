#ifndef POKER_GAMEBOARD_H
#define POKER_GAMEBOARD_H

#include <vector>
#include "Card.h"
#include "Player.h"

enum class GamePhase {
    PREFLOP,
    FLOP,
    TURN,
    RIVER,
    SHOWDOWN
};

class GameBoard {
private:
    std::vector<Card> communityCards;
    GamePhase currentPhase;
    int potAmount;
    int currentBet;
    int smallBlind;
    int bigBlind;
    std::vector<Player*> players;
    int dealerPosition;
    int currentPlayerIndex;

public:
    GameBoard();
    GameBoard(int smallBlindAmount, int bigBlindAmount);
    
    void setPhase(GamePhase phase);
    GamePhase getCurrentPhase() const;
    void advancePhase();
    std::string getPhaseString() const;
    
    void addCommunityCard(const Card& card);
    void addFlopCards(const std::vector<Card>& cards);
    void addTurnCard(const Card& card);
    void addRiverCard(const Card& card);
    std::vector<Card> getCommunityCards() const;
    void clearCommunityCards();
    
    void addToPot(int amount);
    void setPotAmount(int amount);
    int getPotAmount() const;
    void resetPot();
    
    void setCurrentBet(int bet);
    int getCurrentBet() const;
    void setBlinds(int small, int big);
    int getSmallBlind() const;
    int getBigBlind() const;
    
    void addPlayer(Player* player);
    void removePlayer(Player* player);
    std::vector<Player*> getPlayers() const;
    Player* getCurrentPlayer() const;
    void setCurrentPlayer(int index);
    void nextPlayer();
    void setDealerPosition(int position);
    int getDealerPosition() const;
    
    void displayBoard() const;
    void displayCommunityCards() const;
    void displayPot() const;
    
    void resetBoard();
    bool isGameComplete() const;
};

#endif
