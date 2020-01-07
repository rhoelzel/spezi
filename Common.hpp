#pragma once

#include <cstdint>

namespace spezi
{
    using Square = int;
    
    Square constexpr SquaresPerRank = 8;
    Square constexpr SquaresPerFile = 8;
    Square constexpr NumberOfSquares = SquaresPerRank * SquaresPerFile;
}
