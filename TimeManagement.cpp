#include "TimeManagement.hpp"

#include <algorithm>
#include <stdexcept>

namespace spezi
{
    TimePoint getTargetTime(MilliSeconds const timeLeft, MilliSeconds const increment, MilliSeconds const movetime, int const movesPlayed)
    {
        using namespace std::chrono_literals;
        
        if(timeLeft < -1ms || movetime < -1ms || increment < 0ms)
        {
            throw std::runtime_error("invalid negative remaining time, increment and/or movetime specified");       
        }
        
        if(timeLeft == -1ms && movetime == -1ms)
        {
            auto constexpr never = TimePoint::max();   
            return never;
        }     

        auto constexpr averageMovesPerGame = 50;
        auto const expectedMovesToPlay = std::max(averageMovesPerGame - movesPlayed, 10);

        MilliSeconds const allocatedTimeBasedOnTimeLeft = timeLeft != -1ms ? (timeLeft/expectedMovesToPlay) * (expectedMovesToPlay/10) + increment : movetime;
        MilliSeconds const allocatedTimeBasedOnMovetime = movetime != -1ms ? movetime : allocatedTimeBasedOnTimeLeft;

        return std::chrono::steady_clock::now() + (allocatedTimeBasedOnMovetime < allocatedTimeBasedOnTimeLeft ? 
            allocatedTimeBasedOnMovetime : allocatedTimeBasedOnTimeLeft);
    }

    bool enoughTimeForDeeperSearch(TimePoint const targetTimePoint, MilliSeconds const timeSpentAtPreviousDepth)
    {
        auto const timeLeft = std::chrono::duration_cast<MilliSeconds>(targetTimePoint - std::chrono::steady_clock::now());
        return timeLeft >= timeSpentAtPreviousDepth * 10;
    }
}