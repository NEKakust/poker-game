#ifndef POKER_HANDEVALUATOR_H
#define POKER_HANDEVALUATOR_H

#include <vector>
#include <string>
#include "Card.h"

enum class HandRank {
    HIGH_CARD = 0,
    ONE_PAIR = 1,
    TWO_PAIR = 2,
    THREE_OF_A_KIND = 3,
    STRAIGHT = 4,
    FLUSH = 5,
    FULL_HOUSE = 6,
    FOUR_OF_A_KIND = 7,
    STRAIGHT_FLUSH = 8,
    ROYAL_FLUSH = 9
};

struct HandEvaluation {
    HandRank rank;
    std::string handName;
    int rankValue;
    std::vector<int> kickers; // For tie-breaking
};

class HandEvaluator {
public:
    // Main evaluation method
    static HandEvaluation evaluateHand(const std::vector<Card>& hand);
    
    // Individual hand type checks
    static bool isRoyalFlush(const std::vector<Card>& hand);
    static bool isStraightFlush(const std::vector<Card>& hand);
    static bool isFourOfAKind(const std::vector<Card>& hand);
    static bool isFullHouse(const std::vector<Card>& hand);
    static bool isFlush(const std::vector<Card>& hand);
    static bool isStraight(const std::vector<Card>& hand);
    static bool isThreeOfAKind(const std::vector<Card>& hand);
    static bool isTwoPair(const std::vector<Card>& hand);
    static bool isOnePair(const std::vector<Card>& hand);
    
    // Utility methods
    static int getRankValue(const std::string& rank);
    static std::string getHandName(HandRank rank);
    static std::vector<int> getKickers(const std::vector<Card>& hand, HandRank rank);
    
    // Comparison methods
    static int compareHands(const std::vector<Card>& hand1, const std::vector<Card>& hand2);
    static bool isHandBetter(const std::vector<Card>& hand1, const std::vector<Card>& hand2);
    
    // Helper methods
    static std::vector<int> getRankCounts(const std::vector<Card>& hand);
    static std::vector<int> getSuitCounts(const std::vector<Card>& hand);
    static std::vector<int> getSortedRanks(const std::vector<Card>& hand);
    static bool hasConsecutiveRanks(const std::vector<int>& ranks);
    
private:
    static const std::vector<std::string> HAND_NAMES;
};

#endif //POKER_HANDEVALUATOR_H
