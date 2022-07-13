#pragma once

#include <chrono>
#include <string>
#include <iostream>

class ScopedTimer
{
private:
    std::string m_message;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;

public:
    ScopedTimer(std::string message)
    {
        m_message = message;
        m_start_time = std::chrono::high_resolution_clock::now();
    }
    ~ScopedTimer()
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time = end_time - m_start_time;
        std::cout << m_message << " finished in " << time.count() << " seconds." << std::endl;
    }
};

class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;

public:
    Timer()
    {
        tick();
    }
    void tick()
    {
        m_start_time = std::chrono::high_resolution_clock::now();
    }
    void tock(std::string message = "Timer")
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time = end_time - m_start_time;
        std::cout << message << " finished in " << time.count() << " seconds." << std::endl;
    }
};
