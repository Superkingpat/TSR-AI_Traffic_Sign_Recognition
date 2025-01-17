#pragma once
#include "ObjectHandler.h"

class Timer {
private:
    double m_deltaTime;
    double m_timeCounter1;
    double m_timeCounter2;
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_sinceCreation;

public:
    Timer();
    void startTime();
    void update();
    double getDeltaTime() const;
    double getCounter1() const;
    void resetCounter1();
    double getCounter2() const;
    void resetCounter2();
    double getFullTime();
};
