#pragma once

#include <cstdint>

namespace spezi
{
    enum Piece
    {
        PAWN = 0,
        KNIGHT = 1,
        BISHOP = 2,
        ROOK = 3,
        QUEEN = 4,
        KING = 5
    };

    auto constexpr NumberOfPieceTypes = 6;
}