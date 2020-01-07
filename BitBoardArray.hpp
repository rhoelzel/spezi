#pragma once

#include "BitBoard.hpp"

#include "Common.hpp"

#include <array>
#include <utility>

namespace spezi
{  
    using BitBoardArray = std::array<BitBoard, NumberOfSquares>;

    extern BitBoardArray const Ranks;
    extern BitBoardArray const Files;
    extern BitBoardArray const RanksAndFiles;
    extern BitBoardArray const Diagonals;

    extern BitBoardArray const KingMoveAttacks;
    extern BitBoardArray const RookMasks;
    extern BitBoardArray const BishopMasks;
    extern BitBoardArray const KnightMoveAttacks;          

    template <BitBoard bitBoardGenerator(Square const), Square... squares>
    std::array<BitBoard, sizeof...(squares)> constexpr collectBitBoards(std::integer_sequence<Square, squares...>)
    {
        return std::array<BitBoard, sizeof...(squares)> {bitBoardGenerator(squares)...};            
    }
}