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
    
public:
    PokerGameManager();
    void run();
};

PokerGameManager::PokerGameManager() : gameRunning(false) {
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
        gameRunning = true;
        playGame();
    } else {
        cout << "Не удалось запустить игру!" << endl;
    }
}

void PokerGameManager::playGame() {
    while (gameRunning && stateManager.isGameActive()) {
        displayGameState();
        
        if (stateManager.getCurrentPlayerName() == humanPlayer->getName()) {
            handlePlayerAction();
        } else {
            handleBotAction();
        }
        
        // Check if game should continue
        if (stateManager.getCurrentState() == GameState::GAME_OVER) {
            gameRunning = false;
            cout << "\n=== ИГРА ОКОНЧЕНА ===" << endl;
            stateManager.displayGameStatus();
            break;
        }
        
        // Small delay for better user experience
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void PokerGameManager::displayGameState() {
    cout << "\n=== СОСТОЯНИЕ ИГРЫ ===" << endl;
    cout << "Фаза: " << stateManager.getStateString() << endl;
    cout << "Текущий игрок: " << stateManager.getCurrentPlayerName() << endl;
    cout << "Ваш баланс: $" << playerWallet.getBalance() << endl;
    
    if (humanPlayer) {
        humanPlayer->displayHand();
    }
    
    cout << "=====================" << endl;
}

void PokerGameManager::handlePlayerAction() {
    cout << "\n=== ВАШ ХОД ===" << endl;
    cout << "1. Сбросить карты" << endl;
    cout << "2. Пас" << endl;
    cout << "3. Колл" << endl;
    cout << "4. Рейз" << endl;
    cout << "5. Ва-банк" << endl;
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
            stateManager.playerFold(humanPlayer->getName());
            cout << "Вы сбросили карты." << endl;
            break;
        case 2:
            stateManager.playerCheck(humanPlayer->getName());
            cout << "Вы сделали пас." << endl;
            break;
        case 3: {
            int callAmount = 10; // Simplified - should get from game state
            if (playerWallet.canAfford(callAmount)) {
                playerWallet.placeBet(callAmount);
                stateManager.playerCall(humanPlayer->getName(), callAmount);
                cout << "Вы сделали колл на $" << callAmount << "." << endl;
            } else {
                cout << "Недостаточно средств для колла!" << endl;
            }
            break;
        }
        case 4: {
            int raiseAmount;
            cout << "Введите сумму рейза: $";
            if (cin >> raiseAmount && raiseAmount > 0) {
                if (playerWallet.canAfford(raiseAmount)) {
                    playerWallet.placeBet(raiseAmount);
                    stateManager.playerRaise(humanPlayer->getName(), raiseAmount);
                    cout << "Вы сделали рейз на $" << raiseAmount << "." << endl;
                } else {
                    cout << "Недостаточно средств для рейза!" << endl;
                }
            } else {
                cout << "Неверная сумма рейза!" << endl;
            }
            break;
        }
        case 5:
            if (playerWallet.getBalance() > 0) {
                int allInAmount = playerWallet.getBalance();
                playerWallet.placeBet(allInAmount);
                stateManager.playerAllIn(humanPlayer->getName());
                cout << "Вы пошли ва-банк с $" << allInAmount << "!" << endl;
            } else {
                cout << "Нет денег для ва-банка!" << endl;
            }
            break;
        default:
            cout << "Неверный выбор!" << endl;
            return;
    }
    
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