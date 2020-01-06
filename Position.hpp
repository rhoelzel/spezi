#include "BitBoard.hpp"

namespace spezi
{
    struct Position
    {
        BitBoard white;
        BitBoard black;
        BitBoard kings;
        BitBoard queens;
        BitBoard rooks;
        BitBoard bishops;
        BitBoard knights;
        BitBoard pawns;
    };

    constexpr Position StartPosition
    {
        A1|B1|C1|D1|E1|F1|G1|H1|A2|B2|C1|D2|E2|F2|G2|H2,    // white
        A8|B8|C8|D8|E8|F8|G8|H8|A7|B7|C7|D7|E7|F7|G7|H7,    // black
        E1|E8,                                              // kings
        D1|D8,                                              // queens
        A1|H1|A8|H8,                                        // rooks
        C1|F1|C8|F8,                                        // bishops
        B1|G1|B8|G8,                                        // knights
        A2|B2|C2|D2|E2|F2|G2|H2|A7|B7|C7|D7|E7|F7|G7|H7     // pawns
    };
}