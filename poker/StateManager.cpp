#include "StateManager.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>

StateManager::StateManager() : autoSave(false), saveDirectory("saves/") {
    currentSession = nullptr;
    gameSessions.clear();
    playerStates.clear();
    gameSettings.clear();
    
    // Default settings
    gameSettings["small_blind"] = "5";
    gameSettings["big_blind"] = "10";
    gameSettings["max_players"] = "6";
    gameSettings["time_per_turn"] = "30";
}

StateManager::StateManager(const std::string& saveDir) : autoSave(false), saveDirectory(saveDir) {
    currentSession = nullptr;
    gameSessions.clear();
    playerStates.clear();
    gameSettings.clear();
    
    // Default settings
    gameSettings["small_blind"] = "5";
    gameSettings["big_blind"] = "10";
    gameSettings["max_players"] = "6";
    gameSettings["time_per_turn"] = "30";
}

std::string StateManager::generateSessionId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    return "SESSION_" + std::to_string(dis(gen));
}

GameSession* StateManager::createNewSession() {
    GameSession session;
    session.sessionId = generateSessionId();
    session.gameBoard = std::make_shared<GameBoard>();
    session.bank = std::make_shared<Bank>();
    session.result = std::make_shared<Result>();
    session.timer = std::make_shared<Timer>();
    session.betHistory = std::make_shared<BetHistory>();
    
    gameSessions.push_back(session);
    return &gameSessions.back();
}

void StateManager::cleanupSession(GameSession* session) {
    if (session) {
        session->players.clear();
        session->playerWallets.clear();
        session->gameActive = false;
    }
}

bool StateManager::canTransitionTo(GameState from, GameState to) const {
    // Define valid state transitions
    switch (from) {
        case GameState::MENU:
            return to == GameState::WAITING_FOR_PLAYERS;
        case GameState::WAITING_FOR_PLAYERS:
            return to == GameState::PREFLOP || to == GameState::MENU;
        case GameState::PREFLOP:
            return to == GameState::FLOP || to == GameState::SHOWDOWN || to == GameState::GAME_OVER;
        case GameState::FLOP:
            return to == GameState::TURN || to == GameState::SHOWDOWN || to == GameState::GAME_OVER;
        case GameState::TURN:
            return to == GameState::RIVER || to == GameState::SHOWDOWN || to == GameState::GAME_OVER;
        case GameState::RIVER:
            return to == GameState::SHOWDOWN || to == GameState::GAME_OVER;
        case GameState::SHOWDOWN:
            return to == GameState::GAME_OVER || to == GameState::PREFLOP;
        case GameState::GAME_OVER:
            return to == GameState::MENU || to == GameState::PREFLOP;
        case GameState::PAUSED:
            return to == GameState::PREFLOP || to == GameState::FLOP || 
                   to == GameState::TURN || to == GameState::RIVER;
        default:
            return false;
    }
}

void StateManager::executeStateTransition(GameState newState) {
    if (currentSession && canTransitionTo(currentSession->currentState, newState)) {
        currentSession->currentState = newState;
        
        // Execute state-specific actions
        switch (newState) {
            case GameState::PREFLOP:
                if (currentSession->gameBoard) {
                    currentSession->gameBoard->setPhase(GamePhase::PREFLOP);
                }
                break;
            case GameState::FLOP:
                if (currentSession->gameBoard) {
                    currentSession->gameBoard->setPhase(GamePhase::FLOP);
                }
                break;
            case GameState::TURN:
                if (currentSession->gameBoard) {
                    currentSession->gameBoard->setPhase(GamePhase::TURN);
                }
                break;
            case GameState::RIVER:
                if (currentSession->gameBoard) {
                    currentSession->gameBoard->setPhase(GamePhase::RIVER);
                }
                break;
            case GameState::SHOWDOWN:
                if (currentSession->gameBoard) {
                    currentSession->gameBoard->setPhase(GamePhase::SHOWDOWN);
                }
                determineWinner();
                break;
            default:
                break;
        }
        
        autoSaveIfEnabled();
    }
}

void StateManager::updatePlayerStates() {
    if (!currentSession) return;
    
    for (const auto& player : currentSession->players) {
        if (player) {
            // Update player states based on game conditions
            auto it = playerStates.find(player->getName());
            if (it != playerStates.end() && it->second == PlayerState::ACTIVE) {
                // Check if player can still play
                auto walletIt = currentSession->playerWallets.find(player->getName());
                if (walletIt != currentSession->playerWallets.end()) {
                    if (walletIt->second.getBalance() <= 0) {
                        playerStates[player->getName()] = PlayerState::OUT_OF_MONEY;
                    }
                }
            }
        }
    }
}

bool StateManager::allPlayersReady() const {
    if (!currentSession) return false;
    
    int activePlayers = 0;
    for (const auto& player : currentSession->players) {
        if (player) {
            auto it = playerStates.find(player->getName());
            if (it != playerStates.end() && it->second == PlayerState::ACTIVE) {
                activePlayers++;
            }
        }
    }
    
    return activePlayers >= 2; // Minimum 2 players to start
}

bool StateManager::hasActivePlayers() const {
    if (!currentSession) return false;
    
    for (const auto& player : currentSession->players) {
        if (player) {
            auto it = playerStates.find(player->getName());
            if (it != playerStates.end() && it->second == PlayerState::ACTIVE) {
                return true;
            }
        }
    }
    
    return false;
}

void StateManager::advanceGamePhase() {
    if (!currentSession) return;
    
    switch (currentSession->currentState) {
        case GameState::PREFLOP:
            setState(GameState::FLOP);
            break;
        case GameState::FLOP:
            setState(GameState::TURN);
            break;
        case GameState::TURN:
            setState(GameState::RIVER);
            break;
        case GameState::RIVER:
            setState(GameState::SHOWDOWN);
            break;
        default:
            break;
    }
}

void StateManager::handlePlayerAction(const std::string& playerName, const std::string& action, int amount) {
    if (!currentSession) return;
    
    // Record action in bet history
    if (currentSession->betHistory) {
        if (action == "fold") {
            currentSession->betHistory->addFold(playerName, currentSession->currentRound);
        } else if (action == "check") {
            currentSession->betHistory->addCheck(playerName, currentSession->currentRound);
        } else if (action == "call") {
            currentSession->betHistory->addCall(playerName, amount, currentSession->currentRound);
        } else if (action == "raise") {
            currentSession->betHistory->addRaise(playerName, amount, currentSession->currentRound);
        } else if (action == "all_in") {
            currentSession->betHistory->addAllIn(playerName, amount, currentSession->currentRound);
        }
    }
    
    // Update player state
    if (action == "fold") {
        setPlayerState(playerName, PlayerState::FOLDED);
    } else if (action == "all_in") {
        setPlayerState(playerName, PlayerState::ALL_IN);
    }
}

void StateManager::processBettingRound() {
    if (!currentSession) return;
    
    // Process betting logic here
    // This would involve checking if all active players have acted
    // and determining when to advance to the next phase
}

void StateManager::determineWinner() {
    if (!currentSession || !currentSession->result) return;
    
    // Determine winner based on hand evaluation
    // This is a simplified version - in a real implementation,
    // you would use HandEvaluator to compare hands
    
    std::string winner = "Unknown";
    if (currentSession->players.size() > 0) {
        winner = currentSession->players[0]->getName();
    }
    
    currentSession->winner = winner;
}

void StateManager::distributeWinnings() {
    if (!currentSession || !currentSession->bank || !currentSession->result) return;
    
    // Distribute winnings to the winner
    if (!currentSession->winner.empty()) {
        int potAmount = currentSession->bank->getPotAmount();
        
        // Update winner's wallet
        auto walletIt = currentSession->playerWallets.find(currentSession->winner);
        if (walletIt != currentSession->playerWallets.end()) {
            walletIt->second.winBet(potAmount, currentSession->sessionId);
        }
        
        // Distribute through bank
        std::vector<std::string> winners = {currentSession->winner};
        std::vector<int> amounts = {potAmount};
        currentSession->bank->distributeWinnings(winners, amounts);
    }
}

void StateManager::saveGameState() {
    if (!currentSession) return;
    
    std::string filename = saveDirectory + currentSession->sessionId + ".json";
    // Implementation would save game state to file
    // This is a placeholder
}

void StateManager::loadGameState(const std::string& sessionId) {
    std::string filename = saveDirectory + sessionId + ".json";
    // Implementation would load game state from file
    // This is a placeholder
}

void StateManager::autoSaveIfEnabled() {
    if (autoSave && currentSession) {
        saveGameState();
    }
}

std::string StateManager::createNewGame() {
    GameSession* session = createNewSession();
    currentSession = session;
    return session->sessionId;
}

bool StateManager::joinGame(const std::string& sessionId, std::shared_ptr<Player> player) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            session.players.push_back(player);
            playerStates[player->getName()] = PlayerState::WAITING;
            
            // Create wallet for player
            Wallet wallet(player->getName(), 1000);
            session.playerWallets[player->getName()] = wallet;
            
            return true;
        }
    }
    return false;
}

bool StateManager::leaveGame(const std::string& sessionId, const std::string& playerName) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            // Remove player from session
            session.players.erase(
                std::remove_if(session.players.begin(), session.players.end(),
                              [&playerName](const std::shared_ptr<Player>& player) {
                                  return player && player->getName() == playerName;
                              }),
                session.players.end()
            );
            
            // Remove player state
            playerStates.erase(playerName);
            
            // Remove player wallet
            session.playerWallets.erase(playerName);
            
            return true;
        }
    }
    return false;
}

GameSession* StateManager::getCurrentSession() {
    return currentSession;
}

std::vector<GameSession> StateManager::getAllSessions() const {
    return gameSessions;
}

bool StateManager::startGame(const std::string& sessionId) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            if (allPlayersReady()) {
                currentSession = &session;
                session.gameActive = true;
                setState(GameState::PREFLOP);
                return true;
            }
        }
    }
    return false;
}

bool StateManager::pauseGame(const std::string& sessionId) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            setState(GameState::PAUSED);
            return true;
        }
    }
    return false;
}

bool StateManager::resumeGame(const std::string& sessionId) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            // Resume to previous state or PREFLOP
            setState(GameState::PREFLOP);
            return true;
        }
    }
    return false;
}

bool StateManager::endGame(const std::string& sessionId) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            setState(GameState::GAME_OVER);
            session.gameActive = false;
            distributeWinnings();
            return true;
        }
    }
    return false;
}

bool StateManager::resetGame(const std::string& sessionId) {
    for (auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            cleanupSession(&session);
            setState(GameState::MENU);
            return true;
        }
    }
    return false;
}

GameState StateManager::getCurrentState() const {
    return currentSession ? currentSession->currentState : GameState::MENU;
}

void StateManager::setState(GameState newState) {
    if (currentSession) {
        executeStateTransition(newState);
    }
}

std::string StateManager::getStateString() const {
    switch (getCurrentState()) {
        case GameState::MENU: return "Меню";
        case GameState::WAITING_FOR_PLAYERS: return "Ожидание игроков";
        case GameState::PREFLOP: return "Префлоп";
        case GameState::FLOP: return "Флоп";
        case GameState::TURN: return "Терн";
        case GameState::RIVER: return "Ривер";
        case GameState::SHOWDOWN: return "Шоудаун";
        case GameState::GAME_OVER: return "Игра окончена";
        case GameState::PAUSED: return "Пауза";
        default: return "Неизвестно";
    }
}

bool StateManager::isGameActive() const {
    return currentSession && currentSession->gameActive;
}

bool StateManager::playerFold(const std::string& playerName) {
    handlePlayerAction(playerName, "fold", 0);
    return true;
}

bool StateManager::playerCheck(const std::string& playerName) {
    handlePlayerAction(playerName, "check", 0);
    return true;
}

bool StateManager::playerCall(const std::string& playerName, int amount) {
    handlePlayerAction(playerName, "call", amount);
    return true;
}

bool StateManager::playerRaise(const std::string& playerName, int amount) {
    handlePlayerAction(playerName, "raise", amount);
    return true;
}

bool StateManager::playerAllIn(const std::string& playerName) {
    auto walletIt = currentSession->playerWallets.find(playerName);
    if (walletIt != currentSession->playerWallets.end()) {
        int amount = walletIt->second.getBalance();
        handlePlayerAction(playerName, "all_in", amount);
        return true;
    }
    return false;
}

void StateManager::addPlayer(std::shared_ptr<Player> player) {
    if (currentSession && player) {
        currentSession->players.push_back(player);
        playerStates[player->getName()] = PlayerState::WAITING;
        
        // Create wallet for player
        Wallet wallet(player->getName(), 1000);
        currentSession->playerWallets[player->getName()] = wallet;
    }
}

void StateManager::removePlayer(const std::string& playerName) {
    if (currentSession) {
        // Remove player from session
        currentSession->players.erase(
            std::remove_if(currentSession->players.begin(), currentSession->players.end(),
                          [&playerName](const std::shared_ptr<Player>& player) {
                              return player && player->getName() == playerName;
                          }),
            currentSession->players.end()
        );
        
        // Remove player state
        playerStates.erase(playerName);
        
        // Remove player wallet
        currentSession->playerWallets.erase(playerName);
    }
}

std::vector<std::shared_ptr<Player>> StateManager::getPlayers() const {
    return currentSession ? currentSession->players : std::vector<std::shared_ptr<Player>>();
}

std::shared_ptr<Player> StateManager::getPlayer(const std::string& playerName) const {
    if (currentSession) {
        for (const auto& player : currentSession->players) {
            if (player && player->getName() == playerName) {
                return player;
            }
        }
    }
    return nullptr;
}

PlayerState StateManager::getPlayerState(const std::string& playerName) const {
    auto it = playerStates.find(playerName);
    return it != playerStates.end() ? it->second : PlayerState::WAITING;
}

void StateManager::setPlayerState(const std::string& playerName, PlayerState state) {
    playerStates[playerName] = state;
}

int StateManager::getCurrentRound() const {
    return currentSession ? currentSession->currentRound : 0;
}

int StateManager::getDealerPosition() const {
    return currentSession ? currentSession->dealerPosition : 0;
}

int StateManager::getCurrentPlayerIndex() const {
    return currentSession ? currentSession->currentPlayerIndex : 0;
}

std::string StateManager::getCurrentPlayerName() const {
    if (currentSession && currentSession->currentPlayerIndex < currentSession->players.size()) {
        auto player = currentSession->players[currentSession->currentPlayerIndex];
        return player ? player->getName() : "";
    }
    return "";
}

void StateManager::nextPlayer() {
    if (currentSession) {
        currentSession->currentPlayerIndex = 
            (currentSession->currentPlayerIndex + 1) % currentSession->players.size();
    }
}

void StateManager::nextRound() {
    if (currentSession) {
        currentSession->currentRound++;
        advanceGamePhase();
    }
}

void StateManager::setSetting(const std::string& key, const std::string& value) {
    gameSettings[key] = value;
}

std::string StateManager::getSetting(const std::string& key) const {
    auto it = gameSettings.find(key);
    return it != gameSettings.end() ? it->second : "";
}

void StateManager::setAutoSave(bool enable) {
    autoSave = enable;
}

void StateManager::setSaveDirectory(const std::string& dir) {
    saveDirectory = dir;
}

std::map<std::string, int> StateManager::getGameStatistics() const {
    std::map<std::string, int> stats;
    stats["Total Sessions"] = gameSessions.size();
    stats["Active Sessions"] = 0;
    
    for (const auto& session : gameSessions) {
        if (session.gameActive) {
            stats["Active Sessions"]++;
        }
    }
    
    return stats;
}

std::vector<std::string> StateManager::getGameHistory() const {
    std::vector<std::string> history;
    for (const auto& session : gameSessions) {
        history.push_back("Session " + session.sessionId + ": " + 
                         (session.gameActive ? "Active" : "Inactive"));
    }
    return history;
}

void StateManager::displayGameStatus() const {
    std::cout << "\n=== СТАТУС ИГРЫ ===" << std::endl;
    std::cout << "Текущее состояние: " << getStateString() << std::endl;
    std::cout << "Игра активна: " << (isGameActive() ? "Да" : "Нет") << std::endl;
    if (currentSession) {
        std::cout << "ID сессии: " << currentSession->sessionId << std::endl;
        std::cout << "Игроки: " << currentSession->players.size() << std::endl;
        std::cout << "Текущий раунд: " << currentSession->currentRound << std::endl;
    }
    std::cout << "==================" << std::endl;
}

void StateManager::displayPlayerStatus() const {
    std::cout << "\n=== СТАТУС ИГРОКОВ ===" << std::endl;
    for (const auto& playerState : playerStates) {
        std::cout << playerState.first << ": ";
        switch (playerState.second) {
            case PlayerState::WAITING: std::cout << "Ожидание"; break;
            case PlayerState::ACTIVE: std::cout << "Активен"; break;
            case PlayerState::FOLDED: std::cout << "Сбросил карты"; break;
            case PlayerState::ALL_IN: std::cout << "Ва-банк"; break;
            case PlayerState::OUT_OF_MONEY: std::cout << "Без денег"; break;
        }
        std::cout << std::endl;
    }
    std::cout << "====================" << std::endl;
}

void StateManager::cleanup() {
    for (auto& session : gameSessions) {
        cleanupSession(&session);
    }
    gameSessions.clear();
    playerStates.clear();
    currentSession = nullptr;
}

void StateManager::reset() {
    cleanup();
    gameSettings.clear();
    
    // Reset default settings
    gameSettings["small_blind"] = "5";
    gameSettings["big_blind"] = "10";
    gameSettings["max_players"] = "6";
    gameSettings["time_per_turn"] = "30";
}

bool StateManager::isValidSession(const std::string& sessionId) const {
    for (const auto& session : gameSessions) {
        if (session.sessionId == sessionId) {
            return true;
        }
    }
    return false;
}

size_t StateManager::getSessionCount() const {
    return gameSessions.size();
}

void StateManager::onTimerExpired() {
    if (currentSession && currentSession->timer) {
        // Handle timer expiration
        std::string currentPlayer = getCurrentPlayerName();
        if (!currentPlayer.empty()) {
            // Auto-fold current player
            playerFold(currentPlayer);
            nextPlayer();
        }
    }
}

void StateManager::onPlayerDisconnect(const std::string& playerName) {
    setPlayerState(playerName, PlayerState::FOLDED);
}

void StateManager::onGameError(const std::string& error) {
    std::cout << "Game Error: " << error << std::endl;
    // Handle game error - could pause game, log error, etc.
}
