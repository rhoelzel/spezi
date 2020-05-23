#include "TimeManagement.hpp"

#include <algorithm>

namespace spezi
{
    TimePoint getTargetTime(MilliSeconds const timeLeft, MilliSeconds const increment, int const movesPlayed)
    {
        using namespace std::chrono_literals;
        if(timeLeft == -1ms)
        {
            auto constexpr never = TimePoint::max();   
            return never;
        }     

        auto constexpr averageMovesPerGame = 50;
        auto const expectedMovesToPlay = std::max(averageMovesPerGame - movesPlayed, 10);

        MilliSeconds const allocatedTime = (timeLeft/expectedMovesToPlay) * (expectedMovesToPlay/10) + increment;
 
        return std::chrono::steady_clock::now() + allocatedTime;
    }

    bool enoughTimeForDeeperSearch(TimePoint const targetTimePoint, MilliSeconds const timeSpentAtPreviousDepth)
    {
        auto const timeLeft = std::chrono::duration_cast<MilliSeconds>(targetTimePoint - std::chrono::steady_clock::now());
        return timeLeft >= timeSpentAtPreviousDepth * 10;
    }
}