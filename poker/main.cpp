#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <limits>
#include <thread>
#include <chrono>
#include <io.h>
#include <fcntl.h>

#include "Player.h"
#include "BotPlayer.h"
#include "PokerGame.h"
#include "StateManager.h"
#include "Wallet.h"
#include "HandEvaluator.h"

using namespace std;

class PokerGameManager {
private:
    StateManager stateManager;
    shared_ptr<Player> humanPlayer;
    shared_ptr<BotPlayer> botPlayer;
    Wallet playerWallet;
    bool gameRunning;
    Deck gameDeck;
    GameBoard gameBoard;
    int currentBetAmount;
    int potSize;
    int botBalance;
    int playerBetAmount;
    int botBetAmount;
    
    void displayMainMenu();
    void displayGameMenu();
    void displayWalletMenu();
    void displayStatistics();
    void startNewGame();
    void joinExistingGame();
    void playGame();
    void handlePlayerAction();
    void handleBotAction();
    void displayGameState();
    void processGameRound();
    
    void dealCardsToPlayers();
    void displayCommunityCards();
    void displayPlayerPossibleCombinations();
    void dealFlop();
    void dealTurn();
    void dealRiver();
    void determineWinnerAndDistributeWinnings();
    void handlePlayerFold();
    void handlePlayerCheck();
    void handlePlayerCall();
    void handlePlayerRaise();
    void handlePlayerAllIn();
    bool canCheck();
    bool canCall();
    
public:
    PokerGameManager();
    void run();
};

PokerGameManager::PokerGameManager() : gameRunning(false), currentBetAmount(0), potSize(0), botBalance(1000), playerBetAmount(0), botBetAmount(0), gameDeck(), gameBoard() {
    
    stateManager = StateManager();
    playerWallet = Wallet("Player", 1000);
    
    gameDeck.shuffle();
}

void PokerGameManager::run() {
    string playerName;
    cout << "=== ДОБРО ПОЖАЛОВАТЬ В ПОКЕР ===" << endl;
    cout << "Введите ваше имя: ";
    getline(cin, playerName);
    
    if (playerName.empty()) {
        playerName = "Игрок";
    }
    
    humanPlayer = make_shared<Player>(playerName);
    playerWallet.setOwner(playerName);
    botPlayer = make_shared<BotPlayer>("Бот");
    
    cout << "Добро пожаловать, " << playerName << "!" << endl;
    cout << "У вас $" << playerWallet.getBalance() << " в кошельке." << endl;
    
    while (true) {
        displayMainMenu();
        
        int choice;
        cout << "Выберите опцию: ";
        if (!(cin >> choice)) {
            cout << "Неверный ввод! Пожалуйста, введите число." << endl;
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
            continue;
        }

        switch (choice) {
            case 1:
                startNewGame();
                break;
            case 2:
                displayWalletMenu();
                break;
            case 3:
                displayStatistics();
                break;
            case 4:
                cout << "Спасибо за игру!" << endl;
                return;
            default:
                cout << "Неверный выбор! Попробуйте снова." << endl;
                break;
        }
    }
}

void PokerGameManager::displayMainMenu() {
    cout << "\n=== ГЛАВНОЕ МЕНЮ ===" << endl;
    cout << "1. Начать новую игру" << endl;
    cout << "2. Управление кошельком" << endl;
    cout << "3. Статистика" << endl;
    cout << "4. Выход" << endl;
    cout << "===================" << endl;
}

void PokerGameManager::displayWalletMenu() {
    while (true) {
        cout << "\n=== МЕНЮ КОШЕЛЬКА ===" << endl;
        playerWallet.displaySummary();
        cout << "1. Пополнить счет" << endl;
        cout << "2. Снять деньги" << endl;
        cout << "3. История транзакций" << endl;
        cout << "4. Назад в главное меню" << endl;
        cout << "=====================" << endl;
        
        int choice;
        cout << "Выберите опцию: ";
        if (!(cin >> choice)) {
            cout << "Неверный ввод!" << endl;
                        cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
                        continue;
                    }

        switch (choice) {
            case 1: {
                int amount;
                cout << "Введите сумму для пополнения: $";
                if (cin >> amount && amount > 0) {
                    if (playerWallet.deposit(amount)) {
                        cout << "Пополнение успешно!" << endl;
                    }
                } else {
                    cout << "Неверная сумма!" << endl;
                }
                            break;
            }
            case 2: {
                int amount;
                cout << "Введите сумму для снятия: $";
                if (cin >> amount && amount > 0) {
                    if (playerWallet.withdraw(amount)) {
                        cout << "Снятие успешно!" << endl;
                    }
                } else {
                    cout << "Неверная сумма!" << endl;
                }
                break;
            }
            case 3:
                playerWallet.displayRecentTransactions(10);
                break;
            case 4:
                return;
            default:
                cout << "Неверный выбор!" << endl;
                break;
        }
    }
}

void PokerGameManager::displayStatistics() {
    cout << "\n=== СТАТИСТИКА ===" << endl;
    playerWallet.displayStatistics();
    
    if (humanPlayer) {
        cout << "\nСтатистика игр:" << endl;
        cout << "Победы: " << humanPlayer->getWin() << endl;
        cout << "Поражения: " << humanPlayer->getLoss() << endl;
        cout << "Ничьи: " << humanPlayer->getTie() << endl;
    }
    
    cout << "==================" << endl;
}

void PokerGameManager::startNewGame() {
    if (playerWallet.getBalance() < 10) {
        cout << "Недостаточно средств! Минимум $10 требуется для игры." << endl;
        return;
    }
    
    cout << "\n=== НАЧИНАЕМ НОВУЮ ИГРУ ===" << endl;
    string sessionId = stateManager.createNewGame();
    botBalance = 1000;
    cout << "Баланс бота сброшен до $1000" << endl;
    stateManager.addPlayer(humanPlayer);
    stateManager.addPlayer(botPlayer);
    stateManager.setPlayerState(humanPlayer->getName(), PlayerState::ACTIVE);
    stateManager.setPlayerState(botPlayer->getName(), PlayerState::ACTIVE);
    
    if (stateManager.startGame(sessionId)) {
        dealCardsToPlayers();
        gameBoard.resetBoard();
        gameBoard.setBlinds(5, 10);
        
        int smallBlind = 5;
        int bigBlind = 10;
        cout << "\nБлайнды: малый блайнд $" << smallBlind << ", большой блайнд $" << bigBlind << endl;
        
        botBalance -= smallBlind;
        botBetAmount = smallBlind;
        potSize = smallBlind;
        cout << "Бот делает малый блайнд $" << smallBlind << endl;
        cout << "Баланс бота: $" << botBalance << endl;
        
        if (playerWallet.canAfford(bigBlind)) {
            playerWallet.placeBet(bigBlind);
            playerBetAmount = bigBlind;
            potSize = smallBlind + bigBlind;
            cout << "Вы делаете большой блайнд $" << bigBlind << endl;
            cout << "Ваш баланс: $" << playerWallet.getBalance() << endl;
        }
        
        playerBetAmount = 0;
        botBetAmount = 0;
        currentBetAmount = 0;
        
        gameRunning = true;
        playGame();
    } else {
        cout << "Не удалось запустить игру!" << endl;
    }
}

void PokerGameManager::playGame() {
    int roundCount = 0;
    
    while (gameRunning) {
        cout << "\n=== ХОД ===" << endl;
        
        displayGameState();
        handlePlayerAction();
        
        if (gameRunning) {
            handleBotAction();
        }
        
        if (roundCount == 0) {
            cout << "\n=== ФЛОП ===" << endl;
            currentBetAmount = 0;
            dealFlop();
        }
        else if (roundCount == 1) {
            cout << "\n=== ТЕРН ===" << endl;
            currentBetAmount = 0;
            dealTurn();
        }
        else if (roundCount == 2) {
            cout << "\n=== РИВЕР ===" << endl;
            currentBetAmount = 0;
            dealRiver();
            cout << "\n=== ШОУДАУН ===" << endl;
            
            determineWinnerAndDistributeWinnings();
            
            cout << "\n=== ИГРА ОКОНЧЕНА ===" << endl;
            gameRunning = false;
            break;
        }
        
        roundCount++;
        this_thread::sleep_for(chrono::milliseconds(500));
        
        if (!gameRunning) break;
    }
}

void PokerGameManager::displayGameState() {
    cout << "\n=== СОСТОЯНИЕ ИГРЫ ===" << endl;
    cout << "Фаза: " << stateManager.getStateString() << endl;
    cout << "Текущий игрок: " << stateManager.getCurrentPlayerName() << endl;
    cout << "Ваш баланс: $" << playerWallet.getBalance() << endl;
    cout << "Баланс бота: $" << botBalance << endl;
    cout << "Текущая ставка: $" << currentBetAmount << endl;
    cout << "Банк: $" << potSize << endl;
    
    if (humanPlayer) {
        cout << "\n--- Ваши карты ---" << endl;
        humanPlayer->displayHand();
    }
    
    displayCommunityCards();
    
    if (!gameBoard.getCommunityCards().empty()) {
        displayPlayerPossibleCombinations();
    }
    
    cout << "=====================" << endl;
}

void PokerGameManager::handlePlayerAction() {
    cout << "\n=== ВАШ ХОД ===" << endl;
    cout << "1. Сбросить карты (Fold)" << endl;
    
    int menuIndex = 2;
    
    if (canCheck()) {
        cout << menuIndex << ". Чек (Check) - передать ход без ставки" << endl;
    } else if (canCall()) {
        cout << menuIndex << ". Колл (Call) - принять ставку $" << currentBetAmount << endl;
    }
    
    if (playerWallet.getBalance() > 0) {
        cout << "3. Рейз (Raise) - повысить ставку" << endl;
        cout << "4. Ва-банк (All-In)" << endl;
    } else {
        cout << "У вас нет средств для дополнительных ставок (ваш баланс: $0)" << endl;
    }
    cout << "=================" << endl;
    
    int choice;
    cout << "Выберите действие: ";
    if (!(cin >> choice)) {
        cout << "Неверный ввод!" << endl;
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        return;
    }
    
    switch (choice) {
        case 1:
            handlePlayerFold();
            break;
        case 2:
            if (canCheck()) {
                handlePlayerCheck();
            } else if (canCall()) {
                handlePlayerCall();
            } else {
                cout << "Неверный выбор!" << endl;
            }
            break;
        case 3:
            if (playerWallet.getBalance() > 0) {
                handlePlayerRaise();
            } else {
                cout << "У вас нет средств для рейза!" << endl;
            }
            break;
        case 4:
            if (playerWallet.getBalance() > 0) {
                handlePlayerAllIn();
            } else {
                cout << "У вас нет средств для ва-банка!" << endl;
            }
            break;
                        default:
            cout << "Неверный выбор!" << endl;
                            break;
                    }
    
    stateManager.nextPlayer();
}

void PokerGameManager::handleBotAction() {
    cout << "\n=== ХОД БОТА ===" << endl;
    
    if (botPlayer) {
        std::vector<Card> communityCards = gameBoard.getCommunityCards();
        BotDecision decision = botPlayer->getAction(communityCards, potSize, currentBetAmount, 1000);
        botPlayer->displayDecision(decision);
        
        int oldBalance = playerWallet.getBalance();
        switch (decision.action) {
            case BotAction::FOLD:
                cout << "Бот сбрасывает карты." << endl;
                stateManager.playerFold(botPlayer->getName());
                cout << "\n=== БОТ СБРОСИЛ КАРТЫ ===" << endl;
                cout << "=== ВЫ ПОБЕДИЛИ! ===" << endl;
                cout << "Вы выиграли $" << potSize << endl;
                cout << "Старый баланс: $" << oldBalance << endl;
                playerWallet.addBonus(potSize, "Победа в покере (бот сбросил)");
                cout << "Новый баланс: $" << playerWallet.getBalance() << endl;
                gameRunning = false;
                break;
            case BotAction::CHECK:
                cout << "Бот делает чек." << endl;
                stateManager.playerCheck(botPlayer->getName());
                break;
            case BotAction::CALL:
                {
                    int callAmount = playerBetAmount - botBetAmount;
                    if (callAmount > 0) {
                        cout << "Бот делает колл на $" << callAmount << "." << endl;
                        botBalance -= callAmount;
                        botBetAmount += callAmount;
                        potSize += callAmount;
                        currentBetAmount = 0;
                        cout << "Баланс бота после колла: $" << botBalance << ", банк: $" << potSize << endl;
                        stateManager.playerCall(botPlayer->getName(), callAmount);
                    } else {
                        cout << "Ставки уже уравнены, бот делает чек." << endl;
                        stateManager.playerCheck(botPlayer->getName());
                    }
                }
                break;
            case BotAction::RAISE:
                {
                    int raiseAmount = 30;
                    int newBotBet = botBetAmount + raiseAmount;
                    cout << "Бот повышает ставку до $" << newBotBet << " (дополнительно: $" << raiseAmount << ")." << endl;
                    botBalance -= raiseAmount;
                    botBetAmount = newBotBet;
                    potSize += raiseAmount;
                    currentBetAmount = newBotBet - playerBetAmount;
                    cout << "Текущая ставка теперь: $" << currentBetAmount << ", банк: $" << potSize << ", баланс бота: $" << botBalance << endl;
                    stateManager.playerRaise(botPlayer->getName(), newBotBet);
                }
                break;
            case BotAction::ALL_IN:
                {
                    cout << "Бот идет ва-банк!" << endl;
                    int allInAmount = botBalance;
                    potSize += allInAmount;
                    botBalance = 0;
                    cout << "Бот ставит на кон $" << allInAmount << ". Банк: $" << potSize << ", баланс бота: $" << botBalance << endl;
                    stateManager.playerAllIn(botPlayer->getName());
                }
                break;
        }
    }
    
    stateManager.nextPlayer();
}

void PokerGameManager::dealCardsToPlayers() {
    if (humanPlayer) {
        humanPlayer->clearHand();
    }
    if (botPlayer) {
        botPlayer->clearHand();
    }
    
    playerBetAmount = 0;
    botBetAmount = 0;
    currentBetAmount = 0;
    gameDeck.resetDeck();
    gameDeck.shuffle();
    if (humanPlayer) {
        humanPlayer->addCard(gameDeck.dealCard());
        humanPlayer->addCard(gameDeck.dealCard());
    }
    
    if (botPlayer) {
        botPlayer->addCard(gameDeck.dealCard());
        botPlayer->addCard(gameDeck.dealCard());
    }
}

void PokerGameManager::displayCommunityCards() {
    auto cards = gameBoard.getCommunityCards();
    if (!cards.empty()) {
        cout << "\n--- Карты на столе ---" << endl;
        for (size_t i = 0; i < cards.size(); i++) {
            Card card = cards[i];
            cout << "Карта " << (i + 1) << ": " << card.getRank() << " " << card.getSuit() << endl;
        }
        cout << "----------------------" << endl;
    } else {
        cout << "\nКарты на столе еще не выложены" << endl;
    }
}

void PokerGameManager::displayPlayerPossibleCombinations() {
    if (!humanPlayer) return;
    
    std::vector<Card> playerHand = humanPlayer->getHand();
    std::vector<Card> communityCards = gameBoard.getCommunityCards();
    
    if (playerHand.size() < 2) return;
    
    std::vector<Card> fullHand = playerHand;
    fullHand.insert(fullHand.end(), communityCards.begin(), communityCards.end());
    
    if (fullHand.size() < 2) {
        cout << "\n--- Ваши карты: ";
        for (size_t i = 0; i < playerHand.size(); i++) {
            cout << playerHand[i].getRank() << " " << playerHand[i].getSuit();
            if (i < playerHand.size() - 1) cout << ", ";
        }
        cout << " ---" << endl;
        return;
    }
    
    HandEvaluation evaluation = HandEvaluator::evaluateHand(fullHand);
    
    cout << "\n--- Ваши возможные комбинации ---" << endl;
    cout << "Текущая лучшая комбинация: " << evaluation.handName << endl;
    cout << "Сила руки: " << evaluation.rankValue << "/9" << endl;
    
    if (!evaluation.kickers.empty()) {
        cout << "Кикеры: ";
        for (size_t i = 0; i < evaluation.kickers.size() && i < 3; i++) {
            cout << evaluation.kickers[i];
            if (i < evaluation.kickers.size() - 1 && i < 2) cout << ", ";
    }
    cout << endl;
    }
}

void PokerGameManager::dealFlop() {
    if (!gameDeck.isEmpty()) {
        gameDeck.dealCard();
    }
    for (int i = 0; i < 3 && !gameDeck.isEmpty(); i++) {
        Card card = gameDeck.dealCard();
        gameBoard.addCommunityCard(card);
    }
    displayCommunityCards();
}

void PokerGameManager::dealTurn() {
    if (!gameDeck.isEmpty()) {
        gameDeck.dealCard();
    }
    if (!gameDeck.isEmpty()) {
        Card card = gameDeck.dealCard();
        gameBoard.addCommunityCard(card);
    }
    displayCommunityCards();
}

void PokerGameManager::dealRiver() {
    if (!gameDeck.isEmpty()) {
        gameDeck.dealCard();
    }
    if (!gameDeck.isEmpty()) {
        Card card = gameDeck.dealCard();
        gameBoard.addCommunityCard(card);
    }
    displayCommunityCards();
}

void PokerGameManager::determineWinnerAndDistributeWinnings() {
    if (!humanPlayer || !botPlayer) return;
    std::vector<Card> playerHand = humanPlayer->getHand();
    std::vector<Card> botHand = botPlayer->getHand();
    
    std::vector<Card> communityCards = gameBoard.getCommunityCards();
    std::vector<Card> playerFullHand = playerHand;
    playerFullHand.insert(playerFullHand.end(), communityCards.begin(), communityCards.end());
    
    std::vector<Card> botFullHand = botHand;
    botFullHand.insert(botFullHand.end(), communityCards.begin(), communityCards.end());
    
    HandEvaluation playerEval = HandEvaluator::evaluateHand(playerFullHand);
    HandEvaluation botEval = HandEvaluator::evaluateHand(botFullHand);
    cout << "\n--- ОЦЕНКА РУК ---" << endl;
    cout << "Ваша лучшая рука: " << playerEval.handName << endl;
    cout << "Рука бота: " << botEval.handName << endl;
    
    bool playerWins = playerEval.rankValue > botEval.rankValue ||
                     (playerEval.rankValue == botEval.rankValue && 
                      playerEval.kickers > botEval.kickers);
    
    int oldBalance = playerWallet.getBalance();
    
    if (playerWins) {
        cout << "\n=== ВЫ ПОБЕДИЛИ! ===" << endl;
        cout << "Вы выиграли $" << potSize << endl;
        cout << "Старый баланс: $" << oldBalance << endl;
        playerWallet.addBonus(potSize, "Победа в покере");
        cout << "Новый баланс: $" << playerWallet.getBalance() << endl;
    } else if (playerEval.rankValue < botEval.rankValue ||
               (playerEval.rankValue == botEval.rankValue && 
                playerEval.kickers < botEval.kickers)) {
        cout << "\n=== БОТ ПОБЕДИЛ! ===" << endl;
        cout << "Вы проиграли $" << potSize << endl;
    } else {
        cout << "\n=== НИЧЬЯ! ===" << endl;
        cout << "Возврат $" << (potSize / 2) << endl;
        playerWallet.addBonus(potSize / 2, "Ничья");
        cout << "Новый баланс: $" << playerWallet.getBalance() << endl;
    }
    potSize = 0;
}

bool PokerGameManager::canCheck() {
    return currentBetAmount == 0;
}

bool PokerGameManager::canCall() {
    return currentBetAmount > 0;
}

void PokerGameManager::handlePlayerFold() {
    if (!humanPlayer) return;
    
    cout << "\nВы сбросили карты. Вы выбыли из раздачи." << endl;
    stateManager.playerFold(humanPlayer->getName());
    gameRunning = false;
}

void PokerGameManager::handlePlayerCheck() {
    if (!canCheck()) {
        cout << "Невозможно сделать чек! Есть текущая ставка $" << currentBetAmount << ". Используйте Колл вместо Чека." << endl;
        return;
    }
    stateManager.playerCheck(humanPlayer->getName());
    cout << "Вы сделали чек." << endl;
}

void PokerGameManager::handlePlayerCall() {
    if (!humanPlayer) return;
    
    if (!canCall()) {
        cout << "Невозможно сделать колл! Нет текущей ставки. Используйте Чек." << endl;
        return;
    }
    int callAmount = botBetAmount - playerBetAmount;
    
    if (callAmount <= 0) {
        cout << "Ставки уже уравнены!" << endl;
        return;
    }
    
    if (playerWallet.canAfford(callAmount)) {
        playerWallet.placeBet(callAmount);
        playerBetAmount += callAmount;
        stateManager.playerCall(humanPlayer->getName(), callAmount);
        potSize += callAmount;
        currentBetAmount = 0;
        cout << "Вы сделали колл на $" << callAmount << "." << endl;
        cout << "Ваш баланс: $" << playerWallet.getBalance() << endl;
        cout << "Банк: $" << potSize << endl;
    } else {
        cout << "Недостаточно средств для колла! Ваш баланс: $" << playerWallet.getBalance() << endl;
    }
}

void PokerGameManager::handlePlayerRaise() {
    if (playerWallet.getBalance() <= 0) {
        cout << "У вас нет средств для рейза!" << endl;
        return;
    }
    
    int raiseAmount;
    cout << "Введите сумму повышения (минимум $20): ";
    if (!(cin >> raiseAmount)) {
        cout << "Неверный ввод!" << endl;
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        return;
    }
    
    if (raiseAmount < 20) {
        cout << "Минимальный рейз $20!" << endl;
        return;
    }
    
    int newBetAmount = playerBetAmount + raiseAmount;
    
    if (playerWallet.canAfford(newBetAmount)) {
        playerWallet.placeBet(newBetAmount);
        playerBetAmount = newBetAmount;
        stateManager.playerRaise(humanPlayer->getName(), newBetAmount);
        potSize += newBetAmount;
        currentBetAmount = playerBetAmount - botBetAmount; // Разница для бота
        cout << "Вы повысили ставку до $" << playerBetAmount << " (дополнительно: $" << raiseAmount << ")." << endl;
        cout << "Бот должен уравнять ставку!" << endl;
    } else {
        cout << "Недостаточно средств!" << endl;
    }
}

void PokerGameManager::handlePlayerAllIn() {
    int allInAmount = playerWallet.getBalance();
    if (allInAmount > 0) {
        playerWallet.placeBet(allInAmount);
        stateManager.playerAllIn(humanPlayer->getName());
        potSize += allInAmount;
        cout << "Вы пошли ва-банк на $" << allInAmount << "!" << endl;
        
        currentBetAmount = 0;
        cout << "Вы пошли ва-банк. Ожидайте ответа бота и раскрытия карт!" << endl;
    } else {
        cout << "У вас нет средств для ва-банка!" << endl;
    }
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    try {
        PokerGameManager gameManager;
        gameManager.run();
    } catch (const exception& e) {
        cout << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}