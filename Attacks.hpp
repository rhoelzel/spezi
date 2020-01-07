#include "BitBoard.hpp"
#include "Constants.hpp"

#include <array>

namespace spezi
{  
    using Square = int;

    extern std::array<BitBoard, NumberOfSquares> const Ranks;
    extern std::array<BitBoard, NumberOfSquares> const Files;
    extern std::array<BitBoard, NumberOfSquares> const RanksAndFiles;
    extern std::array<BitBoard, NumberOfSquares> const Diagonals;

    extern std::array<BitBoard, NumberOfSquares> const KingMoveAttacks;
    extern std::array<BitBoard, NumberOfSquares> const KnightMoveAttacks;          
}