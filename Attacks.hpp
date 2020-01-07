#include "BitBoard.hpp"
#include "Constants.hpp"

#include <array>

namespace spezi
{  
    using Square = int;

    extern std::array<BitBoard, NumberOfSquares> const KingMoveAttacks;
    extern std::array<BitBoard, NumberOfSquares> const KnightMoveAttacks;          
}