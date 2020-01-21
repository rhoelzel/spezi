#pragma once

#include "Square.hpp"

#include <array>
#include <cstdint>

namespace spezi
{
    using BitBoard = uint_fast64_t;

    BitBoard constexpr EMPTY = 0;

    // note: upper case (like here) => BitBoard, lower case (like a1) => Square index
    BitBoard constexpr A1 = 0x0000000000000001;
    BitBoard constexpr B1 = 0x0000000000000002;
    BitBoard constexpr C1 = 0x0000000000000004;
    BitBoard constexpr D1 = 0x0000000000000008;
    BitBoard constexpr E1 = 0x0000000000000010;
    BitBoard constexpr F1 = 0x0000000000000020;
    BitBoard constexpr G1 = 0x0000000000000040;
    BitBoard constexpr H1 = 0x0000000000000080;

    BitBoard constexpr A2 = 0x0000000000000100;
    BitBoard constexpr B2 = 0x0000000000000200;
    BitBoard constexpr C2 = 0x0000000000000400;
    BitBoard constexpr D2 = 0x0000000000000800;
    BitBoard constexpr E2 = 0x0000000000001000;
    BitBoard constexpr F2 = 0x0000000000002000;
    BitBoard constexpr G2 = 0x0000000000004000;
    BitBoard constexpr H2 = 0x0000000000008000;

    BitBoard constexpr A3 = 0x0000000000010000;
    BitBoard constexpr B3 = 0x0000000000020000;
    BitBoard constexpr C3 = 0x0000000000040000;
    BitBoard constexpr D3 = 0x0000000000080000;
    BitBoard constexpr E3 = 0x0000000000100000;
    BitBoard constexpr F3 = 0x0000000000200000;
    BitBoard constexpr G3 = 0x0000000000400000;
    BitBoard constexpr H3 = 0x0000000000800000;

    BitBoard constexpr A4 = 0x0000000001000000;
    BitBoard constexpr B4 = 0x0000000002000000;
    BitBoard constexpr C4 = 0x0000000004000000;
    BitBoard constexpr D4 = 0x0000000008000000;
    BitBoard constexpr E4 = 0x0000000010000000;
    BitBoard constexpr F4 = 0x0000000020000000;
    BitBoard constexpr G4 = 0x0000000040000000;
    BitBoard constexpr H4 = 0x0000000080000000;

    BitBoard constexpr A5 = 0x0000000100000000;
    BitBoard constexpr B5 = 0x0000000200000000;
    BitBoard constexpr C5 = 0x0000000400000000;
    BitBoard constexpr D5 = 0x0000000800000000;
    BitBoard constexpr E5 = 0x0000001000000000;
    BitBoard constexpr F5 = 0x0000002000000000;
    BitBoard constexpr G5 = 0x0000004000000000;
    BitBoard constexpr H5 = 0x0000008000000000;

    BitBoard constexpr A6 = 0x0000010000000000;
    BitBoard constexpr B6 = 0x0000020000000000;
    BitBoard constexpr C6 = 0x0000040000000000;
    BitBoard constexpr D6 = 0x0000080000000000;
    BitBoard constexpr E6 = 0x0000100000000000;
    BitBoard constexpr F6 = 0x0000200000000000;
    BitBoard constexpr G6 = 0x0000400000000000;
    BitBoard constexpr H6 = 0x0000800000000000;

    BitBoard constexpr A7 = 0x0001000000000000;
    BitBoard constexpr B7 = 0x0002000000000000;
    BitBoard constexpr C7 = 0x0004000000000000;
    BitBoard constexpr D7 = 0x0008000000000000;
    BitBoard constexpr E7 = 0x0010000000000000;
    BitBoard constexpr F7 = 0x0020000000000000;
    BitBoard constexpr G7 = 0x0040000000000000;
    BitBoard constexpr H7 = 0x0080000000000000;

    BitBoard constexpr A8 = 0x0100000000000000;
    BitBoard constexpr B8 = 0x0200000000000000;
    BitBoard constexpr C8 = 0x0400000000000000;
    BitBoard constexpr D8 = 0x0800000000000000;
    BitBoard constexpr E8 = 0x1000000000000000;
    BitBoard constexpr F8 = 0x2000000000000000;
    BitBoard constexpr G8 = 0x4000000000000000;
    BitBoard constexpr H8 = 0x8000000000000000;

    std::array<BitBoard, SquaresPerFile> constexpr RANKS = 
    {
        A1|B1|C1|D1|E1|F1|G1|H1,
        A2|B2|C2|D2|E2|F2|G2|H2,
        A3|B3|C3|D3|E3|F3|G3|H3,
        A4|B4|C4|D4|E4|F4|G4|H4,
        A5|B5|C5|D5|E5|F5|G5|H5,
        A6|B6|C6|D6|E6|F6|G6|H6,
        A7|B7|C7|D7|E7|F7|G7|H7,
        A8|B8|C8|D8|E8|F8|G8|H8
    };

    std::array<BitBoard, SquaresPerRank> constexpr FILES = 
    {
        A1|A2|A3|A4|A5|A6|A7|A8,
        B1|B2|B3|B4|B5|B6|B7|B8,
        C1|C2|C3|C4|C5|C6|C7|C8,
        D1|D2|D3|D4|D5|D6|D7|D8,
        E1|E2|E3|E4|E5|E6|E7|E8,
        F1|F2|F3|F4|F5|F6|F7|F8,
        G1|G2|G3|G4|G5|G6|G7|G8,
        H1|H2|H3|H4|H5|H6|H7|H8
    };

    std::array<BitBoard, NumberOfSquares + 1> constexpr SQUARES =
    {
        A1,B1,C1,D1,E1,F1,G1,H1,
        A2,B2,C2,D2,E2,F2,G2,H2,
        A3,B3,C3,D3,E3,F3,G3,H3,
        A4,B4,C4,D4,E4,F4,G4,H4,
        A5,B5,C5,D5,E5,F5,G5,H5,
        A6,B6,C6,D6,E6,F6,G6,H6,
        A7,B7,C7,D7,E7,F7,G7,H7,
        A8,B8,C8,D8,E8,F8,G8,H8,
	    EMPTY   //=SQUARES[OFF_BOARD]
     };
      
    BitBoard constexpr EDGES = A1|A2|A3|A4|A5|A6|A7|A8|B8|C8|D8|E8|F8|G8|H8|H7|H6|H5|H4|H3|H2|H1|G1|F1|E1|D1|C1|B1;
    BitBoard constexpr INNER = ~EDGES;

    BitBoard pdep(BitBoard const source, BitBoard const mask);
    BitBoard pext(BitBoard const source, BitBoard const mask);
      
    Square constexpr popcount(BitBoard bitBoard)
    {
        return __builtin_popcountll(bitBoard);
    }

    Square constexpr ffs(BitBoard const bitBoard)
    {
        // will return NULL_SQUARE for empty BitBoard
        return NULL_SQUARE + __builtin_ffsll(bitBoard);
    }

    void prettyPrint(BitBoard bitBoard);
}

