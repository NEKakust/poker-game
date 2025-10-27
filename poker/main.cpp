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
#include "Timer.h"
#include "HandEvaluator.h"

using namespace std;

class PokerGameManager {
private:
    StateManager stateManager;
    shared_ptr<Player> humanPlayer;
    shared_ptr<BotPlayer> botPlayer;
    Wallet playerWallet;
    Timer gameTimer;
    bool gameRunning;
    Deck gameDeck;  // Добавляем колоду
    GameBoard gameBoard;  // Добавляем игровое поле
    int currentBetAmount;  // Текущая ставка
    int potSize;  // Размер банка
    int botBalance;  // Баланс бота
    int playerBetAmount;  // Сколько поставил игрок в этом раунде
    int botBetAmount;  // Сколько поставил бот в этом раунде
    
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
    
    // Новые методы для игры
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
    bool canCheck();  // Можно ли сделать чек (проверка, что текущая ставка = 0)
    bool canCall();  // Можно ли сделать колл (есть текущая ставка > 0)
    
public:
    PokerGameManager();
    void run();
};

PokerGameManager::PokerGameManager() : gameRunning(false), currentBetAmount(0), potSize(0), botBalance(1000), playerBetAmount(0), botBetAmount(0), gameDeck(), gameBoard() {
    // Initialize game components
    stateManager = StateManager();
    playerWallet = Wallet("Player", 1000);
    
    // Set up timer callback
    gameTimer.setOnExpireCallback([this]() {
        cout << "\nTime's up! Auto-folding..." << endl;
        if (humanPlayer) {
            stateManager.playerFold(humanPlayer->getName());
        }
    });
    
    // Initialize game deck and shuffle
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
    
    // Create human player
    humanPlayer = make_shared<Player>(playerName);
    playerWallet.setOwner(playerName);
    
    // Create bot player (EASY - самый мягкий и не сбрасывает, HARD - более агрессивный)
    botPlayer = make_shared<BotPlayer>("Бот", BotDifficulty::EASY);
    
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
    
    // Create new game session
    string sessionId = stateManager.createNewGame();
    
    // Сбрасываем баланс бота для новой игры
    botBalance = 1000;
    cout << "Баланс бота сброшен до $1000" << endl;
    
    // Add players to game
    stateManager.addPlayer(humanPlayer);
    stateManager.addPlayer(botPlayer);
    
    // Set players as active
    stateManager.setPlayerState(humanPlayer->getName(), PlayerState::ACTIVE);
    stateManager.setPlayerState(botPlayer->getName(), PlayerState::ACTIVE);
    
    // Start the game
    if (stateManager.startGame(sessionId)) {
        // Раздаем карты игрокам
        dealCardsToPlayers();
        
        // Инициализируем игровое поле
        gameBoard.resetBoard();
        gameBoard.setBlinds(5, 10);
        
        // Делаем начальные ставки блайндов
        int smallBlind = 5;
        int bigBlind = 10;
        cout << "\nБлайнды: малый блайнд $" << smallBlind << ", большой блайнд $" << bigBlind << endl;
        
        // БОТ делает МАЛЫЙ блайнд
        botBalance -= smallBlind;
        botBetAmount = smallBlind;
        potSize = smallBlind;
        cout << "Бот делает малый блайнд $" << smallBlind << endl;
        cout << "Баланс бота: $" << botBalance << endl;
        
        // ИГРОК делает БОЛЬШОЙ блайнд
        if (playerWallet.canAfford(bigBlind)) {
            playerWallet.placeBet(bigBlind);
            playerBetAmount = bigBlind;
            potSize = smallBlind + bigBlind;
            cout << "Вы делаете большой блайнд $" << bigBlind << endl;
            cout << "Ваш баланс: $" << playerWallet.getBalance() << endl;
        }
        
        // После блайндов ставки уравнены, можно продолжать
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
        
        // Ход игрока
        handlePlayerAction();
        
        // Если игрок не сбросил карты, ходит бот
        if (gameRunning) {
            handleBotAction();
        }
        
        // Проверяем, нужно ли выкладывать карты
        // После первого раунда ставок - флоп
        if (roundCount == 0) {
            cout << "\n=== ФЛОП ===" << endl;
            currentBetAmount = 0; // Сбрасываем ставки
            dealFlop();
        }
        // После второго раунда ставок - терн
        else if (roundCount == 1) {
            cout << "\n=== ТЕРН ===" << endl;
            currentBetAmount = 0; // Сбрасываем ставки
            dealTurn();
        }
        // После третьего раунда ставок - ривер
        else if (roundCount == 2) {
            cout << "\n=== РИВЕР ===" << endl;
            currentBetAmount = 0; // Сбрасываем ставки
            dealRiver();
            cout << "\n=== ШОУДАУН ===" << endl;
            
            // Определяем победителя и начисляем выигрыш
            determineWinnerAndDistributeWinnings();
            
            cout << "\n=== ИГРА ОКОНЧЕНА ===" << endl;
            gameRunning = false;
                            break;
        }
        
        roundCount++;
        
        // Small delay for better user experience
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
    
    // Отображаем карты игрока
    if (humanPlayer) {
        cout << "\n--- Ваши карты ---" << endl;
        humanPlayer->displayHand();
    }
    
    // Отображаем карты на столе
    displayCommunityCards();
    
    // Отображаем возможные комбинации игрока
    if (!gameBoard.getCommunityCards().empty()) {
        displayPlayerPossibleCombinations();
    }
    
    cout << "=====================" << endl;
}

void PokerGameManager::handlePlayerAction() {
    cout << "\n=== ВАШ ХОД ===" << endl;
    cout << "1. Сбросить карты (Fold)" << endl;
    
    // Всегда показываем номер 2, но с разными вариантами
    int menuIndex = 2;
    
    if (canCheck()) {
        cout << menuIndex << ". Чек (Check) - передать ход без ставки" << endl;
    } else if (canCall()) {
        cout << menuIndex << ". Колл (Call) - принять ставку $" << currentBetAmount << endl;
    }
    
    // Проверяем, может ли игрок делать рейз или ва-банк
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
            // В зависимости от состояния вызываем чек или колл
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
    
    // После действия игрока переходим к следующему
    stateManager.nextPlayer();
}

void PokerGameManager::handleBotAction() {
    cout << "\n=== ХОД БОТА ===" << endl;
    
    if (botPlayer) {
        // Получаем карты на столе
        std::vector<Card> communityCards = gameBoard.getCommunityCards();
        
        // Вызываем решение бота с реальными параметрами
        BotDecision decision = botPlayer->getAction(communityCards, potSize, currentBetAmount, 1000);
        botPlayer->displayDecision(decision);
        
        // Execute bot action
        int oldBalance = playerWallet.getBalance();
        switch (decision.action) {
            case BotAction::FOLD:
                cout << "Бот сбрасывает карты." << endl;
                stateManager.playerFold(botPlayer->getName());
                // Игрок побеждает автоматически
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
                    // Бот должен уравнять ставку игрока
                    int callAmount = playerBetAmount - botBetAmount;
                    if (callAmount > 0) {
                        cout << "Бот делает колл на $" << callAmount << "." << endl;
                        botBalance -= callAmount;
                        botBetAmount += callAmount;
                        potSize += callAmount;
                        currentBetAmount = 0; // Ставки уравнены
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
                    int raiseAmount = 30; // Минимальный рейз бота
                    int newBotBet = botBetAmount + raiseAmount;
                    cout << "Бот повышает ставку до $" << newBotBet << " (дополнительно: $" << raiseAmount << ")." << endl;
                    botBalance -= raiseAmount;
                    botBetAmount = newBotBet;
                    potSize += raiseAmount;
                    currentBetAmount = newBotBet - playerBetAmount; // Разница для игрока
                    cout << "Текущая ставка теперь: $" << currentBetAmount << ", банк: $" << potSize << ", баланс бота: $" << botBalance << endl;
                    stateManager.playerRaise(botPlayer->getName(), newBotBet);
                }
                break;
            case BotAction::ALL_IN:
                {
                    cout << "Бот идет ва-банк!" << endl;
                    int allInAmount = botBalance; // Используем текущий баланс бота
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

// Реализация новых методов

void PokerGameManager::dealCardsToPlayers() {
    // Очищаем руки игроков
    if (humanPlayer) {
        humanPlayer->clearHand();
    }
    if (botPlayer) {
        botPlayer->clearHand();
    }
    
    // Сбрасываем ставки для новой раздачи
    playerBetAmount = 0;
    botBetAmount = 0;
    currentBetAmount = 0;
    
    // Сбрасываем и тасуем колоду
    gameDeck.resetDeck();
    gameDeck.shuffle();
    
    // Раздаем по 2 карты каждому игроку
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
    
    if (playerHand.size() < 2) return; // У игрока должны быть 2 карты
    
    // Формируем полную руку
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
    
    // Оцениваем руку
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
    // Сжигаем одну карту
    if (!gameDeck.isEmpty()) {
        gameDeck.dealCard();
    }
    
    // Выкладываем 3 карты
    for (int i = 0; i < 3 && !gameDeck.isEmpty(); i++) {
        Card card = gameDeck.dealCard();
        gameBoard.addCommunityCard(card);
    }
    
    // Выводим уже в playGame, поэтому не дублируем здесь
    displayCommunityCards();
}

void PokerGameManager::dealTurn() {
    // Сжигаем одну карту
    if (!gameDeck.isEmpty()) {
        gameDeck.dealCard();
    }
    
    // Выкладываем 1 карту
    if (!gameDeck.isEmpty()) {
        Card card = gameDeck.dealCard();
        gameBoard.addCommunityCard(card);
    }
    
    // Выводим уже в playGame
    displayCommunityCards();
}

void PokerGameManager::dealRiver() {
    // Сжигаем одну карту
    if (!gameDeck.isEmpty()) {
        gameDeck.dealCard();
    }
    
    // Выкладываем 1 карту
    if (!gameDeck.isEmpty()) {
        Card card = gameDeck.dealCard();
        gameBoard.addCommunityCard(card);
    }
    
    // Выводим уже в playGame
    displayCommunityCards();
}

void PokerGameManager::determineWinnerAndDistributeWinnings() {
    if (!humanPlayer || !botPlayer) return;
    
    // Получаем карты игрока и бота
    std::vector<Card> playerHand = humanPlayer->getHand();
    std::vector<Card> botHand = botPlayer->getHand();
    
    // Получаем карты на столе
    std::vector<Card> communityCards = gameBoard.getCommunityCards();
    
    // Формируем полные руки (комбинация карт игрока + карты на столе)
    std::vector<Card> playerFullHand = playerHand;
    playerFullHand.insert(playerFullHand.end(), communityCards.begin(), communityCards.end());
    
    std::vector<Card> botFullHand = botHand;
    botFullHand.insert(botFullHand.end(), communityCards.begin(), communityCards.end());
    
    // Оцениваем руки
    HandEvaluation playerEval = HandEvaluator::evaluateHand(playerFullHand);
    HandEvaluation botEval = HandEvaluator::evaluateHand(botFullHand);
    
    // Отображаем результаты
    cout << "\n--- ОЦЕНКА РУК ---" << endl;
    cout << "Ваша лучшая рука: " << playerEval.handName << endl;
    cout << "Рука бота: " << botEval.handName << endl;
    
    // Сравниваем руки
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
    
    // Обнуляем банк
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
    
    // Помечаем игру как завершенную для игрока
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
    
    // Вычисляем разницу: сколько нужно поставить, чтобы уравнять ставку бота
    int callAmount = botBetAmount - playerBetAmount;
    
    if (callAmount <= 0) {
        cout << "Ставки уже уравнены!" << endl;
        return;
    }
    
    if (playerWallet.canAfford(callAmount)) {
        // Снимаем ставку с кошелька
        playerWallet.placeBet(callAmount);
        playerBetAmount += callAmount;
        // Фиксируем действие в StateManager
        stateManager.playerCall(humanPlayer->getName(), callAmount);
        // Увеличиваем банк
        potSize += callAmount;
        currentBetAmount = 0; // Ставки уравнены
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
        
        // После ва-банка у игрока больше нет средств, он не может делать ход
        // Обнуляем currentBetAmount чтобы бот понимал что ставка покрыта
        currentBetAmount = 0;
        cout << "Вы пошли ва-банк. Ожидайте ответа бота и раскрытия карт!" << endl;
    } else {
        cout << "У вас нет средств для ва-банка!" << endl;
    }
}

int main() {
    // Настройка консоли для отображения UTF-8 текста
    SetConsoleOutputCP(65001);  // UTF-8
    SetConsoleCP(65001);        // UTF-8
    
    try {
        PokerGameManager gameManager;
        gameManager.run();
    } catch (const exception& e) {
        cout << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}