#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace Aimware {

enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    DEBUG,
    SUCCESS
};

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void Initialize(bool console = true, bool file = false) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (initialized_) return;

        if (console) {
            AllocConsole();
            FILE* dummy;
            freopen_s(&dummy, "CONOUT$", "w", stdout);
            freopen_s(&dummy, "CONOUT$", "w", stderr);
            SetConsoleTitleA("Aimware 2016 - Debug Console");
        }
        initialized_ = true;
        Log(LogLevel::SUCCESS, "Logger initialized");
    }

    void Log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] ";

        switch (level) {
            case LogLevel::INFO:    ss << "[INFO] "; break;
            case LogLevel::WARN:    ss << "[WARN] "; SetColor(14); break;
            case LogLevel::ERROR:   ss << "[ERROR] "; SetColor(12); break;
            case LogLevel::DEBUG:   ss << "[DEBUG] "; SetColor(8); break;
            case LogLevel::SUCCESS: ss << "[OK] "; SetColor(10); break;
        }

        ss << message;
        std::cout << ss.str() << std::endl;
        SetColor(7);

        if (level == LogLevel::ERROR) {
            OutputDebugStringA((ss.str() + "\n").c_str());
        }
    }

    template<typename... Args>
    void Info(const std::string& fmt, Args... args) {
        Log(LogLevel::INFO, Format(fmt, args...));
    }

    template<typename... Args>
    void Warn(const std::string& fmt, Args... args) {
        Log(LogLevel::WARN, Format(fmt, args...));
    }

    template<typename... Args>
    void Error(const std::string& fmt, Args... args) {
        Log(LogLevel::ERROR, Format(fmt, args...));
    }

    template<typename... Args>
    void Success(const std::string& fmt, Args... args) {
        Log(LogLevel::SUCCESS, Format(fmt, args...));
    }

private:
    Logger() : initialized_(false) {}
    ~Logger() = default;

    void SetColor(int color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(hConsole, color);
        }
    }

    template<typename... Args>
    std::string Format(const std::string& fmt, Args... args) {
        // Simple formatting - if no args, return as is
        if constexpr (sizeof...(args) == 0) {
            return fmt;
        } else {
            // Use snprintf style for simplicity
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), fmt.c_str(), args...);
            return std::string(buffer);
        }
    }

    std::mutex mutex_;
    bool initialized_;
};

} // namespace Aimware

#define LOG_INFO(msg, ...) Aimware::Logger::Instance().Info(msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) Aimware::Logger::Instance().Warn(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Aimware::Logger::Instance().Error(msg, ##__VA_ARGS__)
#define LOG_SUCCESS(msg, ...) Aimware::Logger::Instance().Success(msg, ##__VA_ARGS__)
