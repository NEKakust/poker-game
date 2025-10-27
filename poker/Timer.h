#ifndef POKER_TIMER_H
#define POKER_TIMER_H

#include <chrono>
#include <functional>
#include <thread>
#include <atomic>
#include <string>

class Timer {
private:
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    int durationSeconds;
    std::atomic<bool> isRunning;
    std::atomic<bool> isExpired;
    std::function<void()> onExpireCallback;
    std::thread timerThread;
    
    void timerLoop();

public:
    Timer();
    Timer(int seconds);
    ~Timer();
    
    // Timer control
    void start();
    void start(int seconds);
    void stop();
    void reset();
    void pause();
    void resume();
    
    // Timer state
    bool isActive() const;
    bool hasExpired() const;
    int getRemainingTime() const;
    int getElapsedTime() const;
    int getDuration() const;
    void setDuration(int seconds);
    
    // Callback management
    void setOnExpireCallback(std::function<void()> callback);
    void clearCallback();
    
    // Display methods
    void displayTime() const;
    void displayCountdown() const;
    std::string getTimeString() const;
    std::string getCountdownString() const;
    
    // Utility methods
    static std::string formatTime(int seconds);
    static int parseTimeString(const std::string& timeStr);
    
    // Copy and move constructors
    Timer(const Timer& other) = delete;
    Timer& operator=(const Timer& other) = delete;
    Timer(Timer&& other) noexcept;
    Timer& operator=(Timer&& other) noexcept;
};

#endif //POKER_TIMER_H
