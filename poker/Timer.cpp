#include "Timer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

Timer::Timer() : durationSeconds(30), isRunning(false), isExpired(false) {
    startTime = std::chrono::steady_clock::now();
    endTime = startTime;
}

Timer::Timer(int seconds) : durationSeconds(seconds), isRunning(false), isExpired(false) {
    startTime = std::chrono::steady_clock::now();
    endTime = startTime;
}

Timer::~Timer() {
    stop();
}

void Timer::start() {
    start(durationSeconds);
}

void Timer::start(int seconds) {
    if (isRunning) {
        stop();
    }
    
    durationSeconds = seconds;
    startTime = std::chrono::steady_clock::now();
    endTime = startTime + std::chrono::seconds(seconds);
    isRunning = true;
    isExpired = false;
    
    // Start timer thread
    timerThread = std::thread(&Timer::timerLoop, this);
}

void Timer::stop() {
    isRunning = false;
    if (timerThread.joinable()) {
        timerThread.join();
    }
}

void Timer::reset() {
    stop();
    isExpired = false;
    startTime = std::chrono::steady_clock::now();
    endTime = startTime;
}

void Timer::pause() {
    isRunning = false;
}

void Timer::resume() {
    if (!isExpired) {
        isRunning = true;
    }
}

bool Timer::isActive() const {
    return isRunning && !isExpired;
}

bool Timer::hasExpired() const {
    return isExpired;
}

int Timer::getRemainingTime() const {
    if (isExpired) return 0;
    
    auto now = std::chrono::steady_clock::now();
    auto remaining = endTime - now;
    int seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
    
    return std::max(0, seconds);
}

int Timer::getElapsedTime() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - startTime;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
}

int Timer::getDuration() const {
    return durationSeconds;
}

void Timer::setDuration(int seconds) {
    durationSeconds = seconds;
}

void Timer::setOnExpireCallback(std::function<void()> callback) {
    onExpireCallback = callback;
}

void Timer::clearCallback() {
    onExpireCallback = nullptr;
}

void Timer::displayTime() const {
    std::cout << "Time: " << getTimeString() << std::endl;
}

void Timer::displayCountdown() const {
    std::cout << "Time remaining: " << getCountdownString() << std::endl;
}

std::string Timer::getTimeString() const {
    int elapsed = getElapsedTime();
    return formatTime(elapsed);
}

std::string Timer::getCountdownString() const {
    int remaining = getRemainingTime();
    return formatTime(remaining);
}

std::string Timer::formatTime(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    std::ostringstream oss;
    if (hours > 0) {
        oss << std::setfill('0') << std::setw(2) << hours << ":";
    }
    oss << std::setfill('0') << std::setw(2) << minutes << ":";
    oss << std::setfill('0') << std::setw(2) << secs;
    
    return oss.str();
}

int Timer::parseTimeString(const std::string& timeStr) {
    // Parse format like "1:30" or "90" or "1:30:45"
    std::istringstream iss(timeStr);
    std::string token;
    std::vector<int> parts;
    
    while (std::getline(iss, token, ':')) {
        parts.push_back(std::stoi(token));
    }
    
    int totalSeconds = 0;
    if (parts.size() == 1) {
        // Just seconds
        totalSeconds = parts[0];
    } else if (parts.size() == 2) {
        // Minutes:Seconds
        totalSeconds = parts[0] * 60 + parts[1];
    } else if (parts.size() == 3) {
        // Hours:Minutes:Seconds
        totalSeconds = parts[0] * 3600 + parts[1] * 60 + parts[2];
    }
    
    return totalSeconds;
}

void Timer::timerLoop() {
    while (isRunning && !isExpired) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto now = std::chrono::steady_clock::now();
        if (now >= endTime) {
            isExpired = true;
            isRunning = false;
            
            if (onExpireCallback) {
                onExpireCallback();
            }
            break;
        }
    }
}

Timer::Timer(Timer&& other) noexcept 
    : startTime(other.startTime), endTime(other.endTime), 
      durationSeconds(other.durationSeconds), isRunning(other.isRunning.load()),
      isExpired(other.isExpired.load()), onExpireCallback(std::move(other.onExpireCallback)),
      timerThread(std::move(other.timerThread)) {
    other.isRunning = false;
    other.isExpired = false;
}

Timer& Timer::operator=(Timer&& other) noexcept {
    if (this != &other) {
        stop();
        
        startTime = other.startTime;
        endTime = other.endTime;
        durationSeconds = other.durationSeconds;
        isRunning = other.isRunning.load();
        isExpired = other.isExpired.load();
        onExpireCallback = std::move(other.onExpireCallback);
        timerThread = std::move(other.timerThread);
        
        other.isRunning = false;
        other.isExpired = false;
    }
    return *this;
}
