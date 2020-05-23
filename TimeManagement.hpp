#pragma once

#include <chrono>

namespace spezi
{
    using TimePoint = decltype(std::chrono::steady_clock::now());
    using MilliSeconds = std::chrono::milliseconds;

    TimePoint getTargetTime(MilliSeconds timeLeft, MilliSeconds increment, int movesPlayed);

    bool enoughTimeForDeeperSearch(TimePoint targetTimePoint, MilliSeconds timeSpentAtPreviousDepth);
}