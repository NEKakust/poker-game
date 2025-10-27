#include "BetHistory.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>

BetHistory::BetHistory() : filename("bet_history.txt"), autoSave(false) {
    history.clear();
}

BetHistory::BetHistory(const std::string& file) : filename(file), autoSave(false) {
    history.clear();
}

void BetHistory::addBet(const std::string& playerName, BetAction action, int amount, int round, const std::string& notes) {
    BetRecord record(playerName, action, amount, round, notes);
    history.push_back(record);
    
    if (autoSave) {
        saveToFile();
    }
}

void BetHistory::addFold(const std::string& playerName, int round) {
    addBet(playerName, BetAction::FOLD, 0, round, "Player folded");
}

void BetHistory::addCheck(const std::string& playerName, int round) {
    addBet(playerName, BetAction::CHECK, 0, round, "Player checked");
}

void BetHistory::addCall(const std::string& playerName, int amount, int round) {
    addBet(playerName, BetAction::CALL, amount, round, "Player called");
}

void BetHistory::addRaise(const std::string& playerName, int amount, int round) {
    addBet(playerName, BetAction::RAISE, amount, round, "Player raised");
}

void BetHistory::addAllIn(const std::string& playerName, int amount, int round) {
    addBet(playerName, BetAction::ALL_IN, amount, round, "Player went all-in");
}

void BetHistory::addBlind(const std::string& playerName, int amount, int round, bool isBigBlind) {
    BetAction action = isBigBlind ? BetAction::BIG_BLIND : BetAction::SMALL_BLIND;
    std::string note = isBigBlind ? "Big blind" : "Small blind";
    addBet(playerName, action, amount, round, note);
}

std::vector<BetRecord> BetHistory::getHistory() const {
    return history;
}

std::vector<BetRecord> BetHistory::getPlayerHistory(const std::string& playerName) const {
    std::vector<BetRecord> playerHistory;
    for (const auto& record : history) {
        if (record.playerName == playerName) {
            playerHistory.push_back(record);
        }
    }
    return playerHistory;
}

std::vector<BetRecord> BetHistory::getRoundHistory(int round) const {
    std::vector<BetRecord> roundHistory;
    for (const auto& record : history) {
        if (record.round == round) {
            roundHistory.push_back(record);
        }
    }
    return roundHistory;
}

std::vector<BetRecord> BetHistory::getPlayerRoundHistory(const std::string& playerName, int round) const {
    std::vector<BetRecord> playerRoundHistory;
    for (const auto& record : history) {
        if (record.playerName == playerName && record.round == round) {
            playerRoundHistory.push_back(record);
        }
    }
    return playerRoundHistory;
}

int BetHistory::getTotalBetsByPlayer(const std::string& playerName) const {
    int total = 0;
    for (const auto& record : history) {
        if (record.playerName == playerName) {
            total += record.amount;
        }
    }
    return total;
}

int BetHistory::getTotalBetsByPlayerInRound(const std::string& playerName, int round) const {
    int total = 0;
    for (const auto& record : history) {
        if (record.playerName == playerName && record.round == round) {
            total += record.amount;
        }
    }
    return total;
}

int BetHistory::getTotalPotInRound(int round) const {
    int total = 0;
    for (const auto& record : history) {
        if (record.round == round) {
            total += record.amount;
        }
    }
    return total;
}

int BetHistory::getPlayerActionCount(const std::string& playerName, BetAction action) const {
    int count = 0;
    for (const auto& record : history) {
        if (record.playerName == playerName && record.action == action) {
            count++;
        }
    }
    return count;
}

double BetHistory::getPlayerAggressionFactor(const std::string& playerName) const {
    int raises = getPlayerActionCount(playerName, BetAction::RAISE);
    int calls = getPlayerActionCount(playerName, BetAction::CALL);
    int checks = getPlayerActionCount(playerName, BetAction::CHECK);
    
    int totalActions = raises + calls + checks;
    if (totalActions == 0) return 0.0;
    
    return static_cast<double>(raises) / totalActions;
}

std::vector<std::string> BetHistory::getMostAggressivePlayers() const {
    std::map<std::string, double> aggressionFactors;
    
    // Get unique player names
    std::vector<std::string> players;
    for (const auto& record : history) {
        if (std::find(players.begin(), players.end(), record.playerName) == players.end()) {
            players.push_back(record.playerName);
        }
    }
    
    // Calculate aggression factors
    for (const auto& player : players) {
        aggressionFactors[player] = getPlayerAggressionFactor(player);
    }
    
    // Sort by aggression factor
    std::sort(players.begin(), players.end(), 
              [&aggressionFactors](const std::string& a, const std::string& b) {
                  return aggressionFactors[a] > aggressionFactors[b];
              });
    
    return players;
}

std::vector<std::string> BetHistory::getTightestPlayers() const {
    std::vector<std::string> aggressive = getMostAggressivePlayers();
    std::reverse(aggressive.begin(), aggressive.end());
    return aggressive;
}

std::map<std::string, int> BetHistory::getPlayerStats(const std::string& playerName) const {
    std::map<std::string, int> stats;
    
    stats["Total Bets"] = getTotalBetsByPlayer(playerName);
    stats["Folds"] = getPlayerActionCount(playerName, BetAction::FOLD);
    stats["Checks"] = getPlayerActionCount(playerName, BetAction::CHECK);
    stats["Calls"] = getPlayerActionCount(playerName, BetAction::CALL);
    stats["Raises"] = getPlayerActionCount(playerName, BetAction::RAISE);
    stats["All-Ins"] = getPlayerActionCount(playerName, BetAction::ALL_IN);
    
    return stats;
}

void BetHistory::saveToFile() const {
    saveToFile(filename);
}

void BetHistory::saveToFile(const std::string& file) const {
    std::ofstream outFile(file);
    if (!outFile.is_open()) {
        std::cout << "Ошибка: Не удалось открыть файл " << file << " для записи" << std::endl;
        return;
    }
    
    outFile << "Player,Action,Amount,Round,Timestamp,Notes" << std::endl;
    
    for (const auto& record : history) {
        outFile << record.playerName << ","
                << getActionString(record.action) << ","
                << record.amount << ","
                << record.round << ","
                << getTimestampString(record.timestamp) << ","
                << record.notes << std::endl;
    }
    
    outFile.close();
}

void BetHistory::loadFromFile() {
    loadFromFile(filename);
}

void BetHistory::loadFromFile(const std::string& file) {
    std::ifstream inFile(file);
    if (!inFile.is_open()) {
        std::cout << "Ошибка: Не удалось открыть файл " << file << " для чтения" << std::endl;
        return;
    }
    
    history.clear();
    std::string line;
    std::getline(inFile, line); // Skip header
    
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 6) {
            BetAction action;
            std::string actionStr = tokens[1];
            if (actionStr == "FOLD") action = BetAction::FOLD;
            else if (actionStr == "CHECK") action = BetAction::CHECK;
            else if (actionStr == "CALL") action = BetAction::CALL;
            else if (actionStr == "RAISE") action = BetAction::RAISE;
            else if (actionStr == "ALL_IN") action = BetAction::ALL_IN;
            else if (actionStr == "SMALL_BLIND") action = BetAction::SMALL_BLIND;
            else if (actionStr == "BIG_BLIND") action = BetAction::BIG_BLIND;
            else continue;
            
            BetRecord record(tokens[0], action, std::stoi(tokens[2]), 
                           std::stoi(tokens[3]), tokens[5]);
            history.push_back(record);
        }
    }
    
    inFile.close();
}

void BetHistory::setAutoSave(bool enable) {
    autoSave = enable;
}

void BetHistory::setFilename(const std::string& file) {
    filename = file;
}

void BetHistory::displayHistory() const {
    std::cout << "\n=== BET HISTORY ===" << std::endl;
    for (const auto& record : history) {
        std::cout << "Round " << record.round << ": " << record.playerName
                  << " " << getActionString(record.action);
        if (record.amount > 0) {
            std::cout << " $" << record.amount;
        }
        if (!record.notes.empty()) {
            std::cout << " (" << record.notes << ")";
        }
        std::cout << " - " << getTimestampString(record.timestamp) << std::endl;
    }
    std::cout << "==================" << std::endl;
}

void BetHistory::displayPlayerHistory(const std::string& playerName) const {
    std::cout << "\n=== " << playerName << " BET HISTORY ===" << std::endl;
    auto playerHistory = getPlayerHistory(playerName);
    for (const auto& record : playerHistory) {
        std::cout << "Round " << record.round << ": " << getActionString(record.action);
        if (record.amount > 0) {
            std::cout << " $" << record.amount;
        }
        if (!record.notes.empty()) {
            std::cout << " (" << record.notes << ")";
        }
        std::cout << " - " << getTimestampString(record.timestamp) << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

void BetHistory::displayRoundHistory(int round) const {
    std::cout << "\n=== ROUND " << round << " HISTORY ===" << std::endl;
    auto roundHistory = getRoundHistory(round);
    for (const auto& record : roundHistory) {
        std::cout << record.playerName << " " << getActionString(record.action);
        if (record.amount > 0) {
            std::cout << " $" << record.amount;
        }
        if (!record.notes.empty()) {
            std::cout << " (" << record.notes << ")";
        }
        std::cout << " - " << getTimestampString(record.timestamp) << std::endl;
    }
    std::cout << "========================" << std::endl;
}

void BetHistory::displayStatistics() const {
    std::cout << "\n=== BETTING STATISTICS ===" << std::endl;
    std::cout << "Total records: " << history.size() << std::endl;
    
    // Get unique players
    std::vector<std::string> players;
    for (const auto& record : history) {
        if (std::find(players.begin(), players.end(), record.playerName) == players.end()) {
            players.push_back(record.playerName);
        }
    }
    
    std::cout << "Players: " << players.size() << std::endl;
    
    for (const auto& player : players) {
        std::cout << "\n" << player << ":" << std::endl;
        auto stats = getPlayerStats(player);
        for (const auto& stat : stats) {
            std::cout << "  " << stat.first << ": " << stat.second << std::endl;
        }
        std::cout << "  Aggression Factor: " << std::fixed << std::setprecision(2) 
                  << getPlayerAggressionFactor(player) << std::endl;
    }
    
    std::cout << "=========================" << std::endl;
}

void BetHistory::displayPlayerStatistics(const std::string& playerName) const {
    std::cout << "\n=== " << playerName << " STATISTICS ===" << std::endl;
    auto stats = getPlayerStats(playerName);
    for (const auto& stat : stats) {
        std::cout << stat.first << ": " << stat.second << std::endl;
    }
    std::cout << "Aggression Factor: " << std::fixed << std::setprecision(2) 
              << getPlayerAggressionFactor(playerName) << std::endl;
    std::cout << "=========================" << std::endl;
}

void BetHistory::clearHistory() {
    history.clear();
}

void BetHistory::clearPlayerHistory(const std::string& playerName) {
    history.erase(
        std::remove_if(history.begin(), history.end(),
                      [&playerName](const BetRecord& record) {
                          return record.playerName == playerName;
                      }),
        history.end()
    );
}

void BetHistory::clearRoundHistory(int round) {
    history.erase(
        std::remove_if(history.begin(), history.end(),
                      [round](const BetRecord& record) {
                          return record.round == round;
                      }),
        history.end()
    );
}

size_t BetHistory::getHistorySize() const {
    return history.size();
}

bool BetHistory::isEmpty() const {
    return history.empty();
}

void BetHistory::exportToCSV(const std::string& filename) const {
    saveToFile(filename);
}

void BetHistory::exportPlayerToCSV(const std::string& playerName, const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cout << "Ошибка: Не удалось открыть файл " << filename << " для записи" << std::endl;
        return;
    }
    
    outFile << "Player,Action,Amount,Round,Timestamp,Notes" << std::endl;
    
    for (const auto& record : history) {
        if (record.playerName == playerName) {
            outFile << record.playerName << ","
                    << getActionString(record.action) << ","
                    << record.amount << ","
                    << record.round << ","
                    << getTimestampString(record.timestamp) << ","
                    << record.notes << std::endl;
        }
    }
    
    outFile.close();
}

std::string BetHistory::getActionString(BetAction action) const {
    switch (action) {
        case BetAction::FOLD: return "FOLD";
        case BetAction::CHECK: return "CHECK";
        case BetAction::CALL: return "CALL";
        case BetAction::RAISE: return "RAISE";
        case BetAction::ALL_IN: return "ALL_IN";
        case BetAction::SMALL_BLIND: return "SMALL_BLIND";
        case BetAction::BIG_BLIND: return "BIG_BLIND";
        default: return "UNKNOWN";
    }
}

std::string BetHistory::getTimestampString(const std::chrono::system_clock::time_point& time) const {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
