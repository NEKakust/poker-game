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
    void dealFlop();
    void dealTurn();
    void dealRiver();
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

PokerGameManager::PokerGameManager() : gameRunning(false), currentBetAmount(0), potSize(0), gameDeck(), gameBoard() {
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
    
    // Create bot player
    botPlayer = make_shared<BotPlayer>("Бот", BotDifficulty::MEDIUM);
    
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
        
        // Игрок делает большой блайнд
        if (playerWallet.canAfford(bigBlind)) {
            playerWallet.placeBet(bigBlind);
            potSize += bigBlind;
            cout << "Вы делаете большой блайнд $" << bigBlind << endl;
        }
        
        currentBetAmount = bigBlind;
        potSize = bigBlind;
        
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
    cout << "Текущая ставка: $" << currentBetAmount << endl;
    cout << "Банк: $" << potSize << endl;
    
    // Отображаем карты игрока
    if (humanPlayer) {
        cout << "\n--- Ваши карты ---" << endl;
        humanPlayer->displayHand();
    }
    
    // Отображаем карты на столе
    displayCommunityCards();
    
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
    
    cout << "3. Рейз (Raise) - повысить ставку" << endl;
    cout << "4. Ва-банк (All-In)" << endl;
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
            handlePlayerRaise();
            break;
        case 4:
            handlePlayerAllIn();
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
    
    // Get bot decision (simplified)
    if (botPlayer) {
        BotDecision decision = botPlayer->getAction({}, 100, 10, 10); // Simplified parameters
        botPlayer->displayDecision(decision);
        
        // Execute bot action
        switch (decision.action) {
            case BotAction::FOLD:
                stateManager.playerFold(botPlayer->getName());
                break;
            case BotAction::CHECK:
                stateManager.playerCheck(botPlayer->getName());
                break;
            case BotAction::CALL:
                stateManager.playerCall(botPlayer->getName(), decision.amount);
                break;
            case BotAction::RAISE:
                stateManager.playerRaise(botPlayer->getName(), decision.amount);
                break;
            case BotAction::ALL_IN:
                stateManager.playerAllIn(botPlayer->getName());
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
    
    cout << "\n=== ФЛОП ===" << endl;
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
    
    cout << "\n=== ТЕРН ===" << endl;
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
    
    cout << "\n=== РИВЕР ===" << endl;
    displayCommunityCards();
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
    
    if (playerWallet.canAfford(currentBetAmount)) {
        // Снимаем ставку с кошелька
        playerWallet.placeBet(currentBetAmount);
        // Фиксируем действие в StateManager
        stateManager.playerCall(humanPlayer->getName(), currentBetAmount);
        // Увеличиваем банк
        potSize += currentBetAmount;
        cout << "Вы сделали колл на $" << currentBetAmount << "." << endl;
        cout << "Ваш баланс: $" << playerWallet.getBalance() << endl;
        cout << "Банк: $" << potSize << endl;
    } else {
        cout << "Недостаточно средств для колла! Ваш баланс: $" << playerWallet.getBalance() << endl;
    }
}

void PokerGameManager::handlePlayerRaise() {
    int raiseAmount;
    cout << "Введите сумму повышения (минимум $" << (currentBetAmount + 10) << "): ";
    if (!(cin >> raiseAmount)) {
        cout << "Неверный ввод!" << endl;
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        return;
    }
    
    int totalBet = currentBetAmount + raiseAmount;
    if (playerWallet.canAfford(totalBet)) {
        playerWallet.placeBet(totalBet);
        stateManager.playerRaise(humanPlayer->getName(), totalBet);
        currentBetAmount = totalBet;
        potSize += totalBet;
        cout << "Вы повысили ставку до $" << totalBet << "." << endl;
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