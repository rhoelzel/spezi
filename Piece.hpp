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

    char const * const PieceTags[] =
    {
        "",
        "N",
        "B",
        "R",
        "Q",
        "K"
    };

    auto constexpr NumberOfPieceTypes = 6;
}