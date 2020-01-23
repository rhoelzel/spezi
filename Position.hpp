#pragma once

#include "BitBoard.hpp"
#include "Color.hpp"
#include "Piece.hpp"

namespace spezi
{
    struct Position
    {
        BitBoard empty;
        BitBoard allPieces[NumberOfColors];
        BitBoard individualPieces[NumberOfPieceTypes];
    };

    constexpr Position StartPosition
    {
        A3|B3|C3|D3|E3|F3|G3|H3|A4|B4|C4|D4|E4|F4|G4|H4         // empty
        |A5|B5|C5|D5|E5|F5|G5|H5|A6|B6|C6|D6|E6|F6|G6|H6,
        {
            A1|B1|C1|D1|E1|F1|G1|H1|A2|B2|C2|D2|E2|F2|G2|H2,    // white
            A8|B8|C8|D8|E8|F8|G8|H8|A7|B7|C7|D7|E7|F7|G7|H7     // black
        },
        {
            A2|B2|C2|D2|E2|F2|G2|H2|A7|B7|C7|D7|E7|F7|G7|H7,    // pawns
            B1|G1|B8|G8,                                        // knights
            C1|F1|C8|F8,                                        // bishops
            A1|H1|A8|H8,                                        // rooks
            D1|D8,                                              // queens
            E1|E8,                                              // kings
        }
    };

    void prettyPrint(Position const & position);
}