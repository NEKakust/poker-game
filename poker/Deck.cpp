#include "Deck.h"
#include <ctime>

Deck::Deck() {
    std::string ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Валет", "Дама", "Король", "Туз" };
    std::string suits[] = { "Пики", "Червы", "Бубны", "Трефы" };

    for (const auto & suit : suits) {
        for (const auto & rank : ranks) {
            Card card(rank, suit);
            cards.push_back(card);
        }
    }
}

void Deck::resetDeck() {
    cards.clear();
    std::string ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Валет", "Дама", "Король", "Туз" };
    std::string suits[] = { "Пики", "Червы", "Бубны", "Трефы" };

    for (const auto & suit : suits) {
        for (const auto & rank : ranks) {
            Card card(rank, suit);
            cards.push_back(card);
        }
    }
    shuffle();
}

void Deck::shuffle() {
    srand(time(nullptr));
    for (auto & card : cards) {
        unsigned long long j = rand() % cards.size();
        std::swap(card, cards[j]);
    }

}

Card Deck::dealCard() {
    Card card = cards.back();
    cards.pop_back();
    return card;
}