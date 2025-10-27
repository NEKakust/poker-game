#ifndef POKER_STATEMANAGER_H
#define POKER_STATEMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Player.h"
#include "BotPlayer.h"
#include "GameBoard.h"
#include "Bank.h"
#include "Result.h"
#include "Timer.h"
#include "BetHistory.h"
#include "Wallet.h"

enum class GameState {
    MENU,
    WAITING_FOR_PLAYERS,
    PREFLOP,
    FLOP,
    TURN,
    RIVER,
    SHOWDOWN,
    GAME_OVER,
    PAUSED
};

enum class PlayerState {
    WAITING,
    ACTIVE,
    FOLDED,
    ALL_IN,
    OUT_OF_MONEY
};

struct GameSession {
    std::string sessionId;
    std::vector<std::shared_ptr<Player>> players;
    std::shared_ptr<GameBoard> gameBoard;
    std::shared_ptr<Bank> bank;
    std::shared_ptr<Result> result;
    std::shared_ptr<Timer> timer;
    std::shared_ptr<BetHistory> betHistory;
    std::map<std::string, Wallet> playerWallets;
    GameState currentState;
    int currentRound;
    int dealerPosition;
    int currentPlayerIndex;
    std::string winner;
    bool gameActive;
    
    GameSession() : currentState(GameState::MENU), currentRound(0), 
                   dealerPosition(0), currentPlayerIndex(0), gameActive(false) {}
};

class StateManager {
private:
    std::vector<GameSession> gameSessions;
    GameSession* currentSession;
    std::map<std::string, PlayerState> playerStates;
    std::map<std::string, std::string> gameSettings;
    bool autoSave;
    std::string saveDirectory;
    
    // Session management
    std::string generateSessionId();
    GameSession* createNewSession();
    void cleanupSession(GameSession* session);
    
    // State transitions
    bool canTransitionTo(GameState from, GameState to) const;
    void executeStateTransition(GameState newState);
    
    // Player management
    void updatePlayerStates();
    bool allPlayersReady() const;
    bool hasActivePlayers() const;
    
    // Game flow control
    void advanceGamePhase();
    void handlePlayerAction(const std::string& playerName, const std::string& action, int amount = 0);
    void processBettingRound();
    void determineWinner();
    void distributeWinnings();
    
    // Persistence
    void saveGameState();
    void loadGameState(const std::string& sessionId);
    void autoSaveIfEnabled();

public:
    StateManager();
    StateManager(const std::string& saveDir);
    
    // Session management
    std::string createNewGame();
    bool joinGame(const std::string& sessionId, std::shared_ptr<Player> player);
    bool leaveGame(const std::string& sessionId, const std::string& playerName);
    GameSession* getCurrentSession();
    std::vector<GameSession> getAllSessions() const;
    
    // Game control
    bool startGame(const std::string& sessionId);
    bool pauseGame(const std::string& sessionId);
    bool resumeGame(const std::string& sessionId);
    bool endGame(const std::string& sessionId);
    bool resetGame(const std::string& sessionId);
    
    // State management
    GameState getCurrentState() const;
    void setState(GameState newState);
    std::string getStateString() const;
    bool isGameActive() const;
    
    // Player actions
    bool playerFold(const std::string& playerName);
    bool playerCheck(const std::string& playerName);
    bool playerCall(const std::string& playerName, int amount);
    bool playerRaise(const std::string& playerName, int amount);
    bool playerAllIn(const std::string& playerName);
    
    // Player management
    void addPlayer(std::shared_ptr<Player> player);
    void removePlayer(const std::string& playerName);
    std::vector<std::shared_ptr<Player>> getPlayers() const;
    std::shared_ptr<Player> getPlayer(const std::string& playerName) const;
    PlayerState getPlayerState(const std::string& playerName) const;
    void setPlayerState(const std::string& playerName, PlayerState state);
    
    // Game information
    int getCurrentRound() const;
    int getDealerPosition() const;
    int getCurrentPlayerIndex() const;
    std::string getCurrentPlayerName() const;
    void nextPlayer();
    void nextRound();
    
    // Settings management
    void setSetting(const std::string& key, const std::string& value);
    std::string getSetting(const std::string& key) const;
    void setAutoSave(bool enable);
    void setSaveDirectory(const std::string& dir);
    
    // Statistics and history
    std::map<std::string, int> getGameStatistics() const;
    std::vector<std::string> getGameHistory() const;
    void displayGameStatus() const;
    void displayPlayerStatus() const;
    
    // Utility methods
    void cleanup();
    void reset();
    bool isValidSession(const std::string& sessionId) const;
    size_t getSessionCount() const;
    
    // Event handling
    void onTimerExpired();
    void onPlayerDisconnect(const std::string& playerName);
    void onGameError(const std::string& error);
};

#endif //POKER_STATEMANAGER_H
