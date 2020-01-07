#pragma once

#include "BitBoard.hpp"
#include "Constants.hpp"

#include <array>

namespace spezi
{  
    using Square = int;
    using BitBoardArray = std::array<BitBoard, NumberOfSquares>;

    extern BitBoardArray const Ranks;
    extern BitBoardArray const Files;
    extern BitBoardArray const RanksAndFiles;
    extern BitBoardArray const Diagonals;

    extern BitBoardArray const KingMoveAttacks;
    extern BitBoardArray const RookMasks;
    extern BitBoardArray const BishopMasks;
    extern BitBoardArray const KnightMoveAttacks;          
}