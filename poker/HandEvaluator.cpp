#include "HandEvaluator.h"
#include <algorithm>
#include <map>
#include <iostream>

const std::vector<std::string> HandEvaluator::HAND_NAMES = {
    "High Card",
    "One Pair",
    "Two Pair",
    "Three of a Kind",
    "Straight",
    "Flush",
    "Full House",
    "Four of a Kind",
    "Straight Flush",
    "Royal Flush"
};

HandEvaluation HandEvaluator::evaluateHand(const std::vector<Card>& hand) {
    HandEvaluation evaluation;
    
    if (isRoyalFlush(hand)) {
        evaluation.rank = HandRank::ROYAL_FLUSH;
        evaluation.rankValue = 9;
    } else if (isStraightFlush(hand)) {
        evaluation.rank = HandRank::STRAIGHT_FLUSH;
        evaluation.rankValue = 8;
    } else if (isFourOfAKind(hand)) {
        evaluation.rank = HandRank::FOUR_OF_A_KIND;
        evaluation.rankValue = 7;
    } else if (isFullHouse(hand)) {
        evaluation.rank = HandRank::FULL_HOUSE;
        evaluation.rankValue = 6;
    } else if (isFlush(hand)) {
        evaluation.rank = HandRank::FLUSH;
        evaluation.rankValue = 5;
    } else if (isStraight(hand)) {
        evaluation.rank = HandRank::STRAIGHT;
        evaluation.rankValue = 4;
    } else if (isThreeOfAKind(hand)) {
        evaluation.rank = HandRank::THREE_OF_A_KIND;
        evaluation.rankValue = 3;
    } else if (isTwoPair(hand)) {
        evaluation.rank = HandRank::TWO_PAIR;
        evaluation.rankValue = 2;
    } else if (isOnePair(hand)) {
        evaluation.rank = HandRank::ONE_PAIR;
        evaluation.rankValue = 1;
    } else {
        evaluation.rank = HandRank::HIGH_CARD;
        evaluation.rankValue = 0;
    }
    
    evaluation.handName = getHandName(evaluation.rank);
    evaluation.kickers = getKickers(hand, evaluation.rank);
    
    return evaluation;
}

bool HandEvaluator::isRoyalFlush(const std::vector<Card>& hand) {
    if (!isFlush(hand)) return false;
    
    std::vector<int> ranks = getSortedRanks(hand);
    std::vector<int> royalRanks = {10, 11, 12, 13, 14}; // 10, J, Q, K, A
    
    return std::includes(ranks.begin(), ranks.end(), royalRanks.begin(), royalRanks.end());
}

bool HandEvaluator::isStraightFlush(const std::vector<Card>& hand) {
    return isFlush(hand) && isStraight(hand);
}

bool HandEvaluator::isFourOfAKind(const std::vector<Card>& hand) {
    std::vector<int> rankCounts = getRankCounts(hand);
    return std::find(rankCounts.begin(), rankCounts.end(), 4) != rankCounts.end();
}

bool HandEvaluator::isFullHouse(const std::vector<Card>& hand) {
    std::vector<int> rankCounts = getRankCounts(hand);
    bool hasThree = std::find(rankCounts.begin(), rankCounts.end(), 3) != rankCounts.end();
    bool hasPair = std::find(rankCounts.begin(), rankCounts.end(), 2) != rankCounts.end();
    return hasThree && hasPair;
}

bool HandEvaluator::isFlush(const std::vector<Card>& hand) {
    std::vector<int> suitCounts = getSuitCounts(hand);
    return std::find(suitCounts.begin(), suitCounts.end(), 5) != suitCounts.end();
}

bool HandEvaluator::isStraight(const std::vector<Card>& hand) {
    std::vector<int> ranks = getSortedRanks(hand);
    
    // Check for A-2-3-4-5 straight
    std::vector<int> lowStraight = {2, 3, 4, 5, 14};
    if (std::includes(ranks.begin(), ranks.end(), lowStraight.begin(), lowStraight.end())) {
        return true;
    }
    
    return hasConsecutiveRanks(ranks);
}

bool HandEvaluator::isThreeOfAKind(const std::vector<Card>& hand) {
    std::vector<int> rankCounts = getRankCounts(hand);
    return std::find(rankCounts.begin(), rankCounts.end(), 3) != rankCounts.end();
}

bool HandEvaluator::isTwoPair(const std::vector<Card>& hand) {
    std::vector<int> rankCounts = getRankCounts(hand);
    int pairCount = std::count(rankCounts.begin(), rankCounts.end(), 2);
    return pairCount == 2;
}

bool HandEvaluator::isOnePair(const std::vector<Card>& hand) {
    std::vector<int> rankCounts = getRankCounts(hand);
    return std::find(rankCounts.begin(), rankCounts.end(), 2) != rankCounts.end();
}

int HandEvaluator::getRankValue(const std::string& rank) {
    if (rank == "2") return 2;
    if (rank == "3") return 3;
    if (rank == "4") return 4;
    if (rank == "5") return 5;
    if (rank == "6") return 6;
    if (rank == "7") return 7;
    if (rank == "8") return 8;
    if (rank == "9") return 9;
    if (rank == "10") return 10;
    if (rank == "J") return 11;
    if (rank == "Q") return 12;
    if (rank == "K") return 13;
    if (rank == "A") return 14;
    return 0;
}

std::string HandEvaluator::getHandName(HandRank rank) {
    int index = static_cast<int>(rank);
    if (index >= 0 && index < HAND_NAMES.size()) {
        return HAND_NAMES[index];
    }
    return "Unknown Hand";
}

std::vector<int> HandEvaluator::getKickers(const std::vector<Card>& hand, HandRank rank) {
    std::vector<int> kickers;
    std::vector<int> ranks = getSortedRanks(hand);
    
    switch (rank) {
        case HandRank::HIGH_CARD:
            kickers = ranks;
            break;
        case HandRank::ONE_PAIR:
            // Find pair rank and remaining cards
            {
                std::map<int, int> rankCounts;
                for (int rank : ranks) {
                    rankCounts[rank]++;
                }
                for (auto& pair : rankCounts) {
                    if (pair.second == 2) {
                        kickers.push_back(pair.first);
                        kickers.push_back(pair.first);
                    }
                }
                for (auto& pair : rankCounts) {
                    if (pair.second == 1) {
                        kickers.push_back(pair.first);
                    }
                }
            }
            break;
        case HandRank::TWO_PAIR:
        case HandRank::THREE_OF_A_KIND:
        case HandRank::FOUR_OF_A_KIND:
        case HandRank::FULL_HOUSE:
            // Similar logic for other hand types
            kickers = ranks;
            break;
        default:
            kickers = ranks;
            break;
    }
    
    return kickers;
}

int HandEvaluator::compareHands(const std::vector<Card>& hand1, const std::vector<Card>& hand2) {
    HandEvaluation eval1 = evaluateHand(hand1);
    HandEvaluation eval2 = evaluateHand(hand2);
    
    if (eval1.rankValue > eval2.rankValue) return 1;
    if (eval1.rankValue < eval2.rankValue) return -1;
    
    // Same hand rank, compare kickers
    for (size_t i = 0; i < std::min(eval1.kickers.size(), eval2.kickers.size()); i++) {
        if (eval1.kickers[i] > eval2.kickers[i]) return 1;
        if (eval1.kickers[i] < eval2.kickers[i]) return -1;
    }
    
    return 0; // Tie
}

bool HandEvaluator::isHandBetter(const std::vector<Card>& hand1, const std::vector<Card>& hand2) {
    return compareHands(hand1, hand2) > 0;
}

std::vector<int> HandEvaluator::getRankCounts(const std::vector<Card>& hand) {
    std::map<int, int> rankCounts;
    for (const Card& card : hand) {
        int rank = getRankValue(card.getRank());
        rankCounts[rank]++;
    }
    
    std::vector<int> counts;
    for (auto& pair : rankCounts) {
        counts.push_back(pair.second);
    }
    return counts;
}

std::vector<int> HandEvaluator::getSuitCounts(const std::vector<Card>& hand) {
    std::map<std::string, int> suitCounts;
    for (const Card& card : hand) {
        suitCounts[card.getSuit()]++;
    }
    
    std::vector<int> counts;
    for (auto& pair : suitCounts) {
        counts.push_back(pair.second);
    }
    return counts;
}

std::vector<int> HandEvaluator::getSortedRanks(const std::vector<Card>& hand) {
    std::vector<int> ranks;
    for (const Card& card : hand) {
        ranks.push_back(getRankValue(card.getRank()));
    }
    std::sort(ranks.begin(), ranks.end(), std::greater<int>());
    return ranks;
}

bool HandEvaluator::hasConsecutiveRanks(const std::vector<int>& ranks) {
    if (ranks.size() < 5) return false;
    
    for (size_t i = 0; i <= ranks.size() - 5; i++) {
        bool consecutive = true;
        for (size_t j = 1; j < 5; j++) {
            if (ranks[i + j] != ranks[i] - j) {
                consecutive = false;
                break;
            }
        }
        if (consecutive) return true;
    }
    return false;
}
