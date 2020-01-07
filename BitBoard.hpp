#pragma once

#include "Common.hpp"

#include <array>
#include <cstdint>

namespace spezi
{
    using BitBoard = uint64_t;

    constexpr BitBoard EMPTY = 0;

    constexpr BitBoard A1 = 0x0000000000000001;
    constexpr BitBoard B1 = 0x0000000000000002;
    constexpr BitBoard C1 = 0x0000000000000004;
    constexpr BitBoard D1 = 0x0000000000000008;
    constexpr BitBoard E1 = 0x0000000000000010;
    constexpr BitBoard F1 = 0x0000000000000020;
    constexpr BitBoard G1 = 0x0000000000000040;
    constexpr BitBoard H1 = 0x0000000000000080;

    constexpr BitBoard A2 = 0x0000000000000100;
    constexpr BitBoard B2 = 0x0000000000000200;
    constexpr BitBoard C2 = 0x0000000000000400;
    constexpr BitBoard D2 = 0x0000000000000800;
    constexpr BitBoard E2 = 0x0000000000001000;
    constexpr BitBoard F2 = 0x0000000000002000;
    constexpr BitBoard G2 = 0x0000000000004000;
    constexpr BitBoard H2 = 0x0000000000008000;

    constexpr BitBoard A3 = 0x0000000000010000;
    constexpr BitBoard B3 = 0x0000000000020000;
    constexpr BitBoard C3 = 0x0000000000040000;
    constexpr BitBoard D3 = 0x0000000000080000;
    constexpr BitBoard E3 = 0x0000000000100000;
    constexpr BitBoard F3 = 0x0000000000200000;
    constexpr BitBoard G3 = 0x0000000000400000;
    constexpr BitBoard H3 = 0x0000000000800000;

    constexpr BitBoard A4 = 0x0000000001000000;
    constexpr BitBoard B4 = 0x0000000002000000;
    constexpr BitBoard C4 = 0x0000000004000000;
    constexpr BitBoard D4 = 0x0000000008000000;
    constexpr BitBoard E4 = 0x0000000010000000;
    constexpr BitBoard F4 = 0x0000000020000000;
    constexpr BitBoard G4 = 0x0000000040000000;
    constexpr BitBoard H4 = 0x0000000080000000;

    constexpr BitBoard A5 = 0x0000000100000000;
    constexpr BitBoard B5 = 0x0000000200000000;
    constexpr BitBoard C5 = 0x0000000400000000;
    constexpr BitBoard D5 = 0x0000000800000000;
    constexpr BitBoard E5 = 0x0000001000000000;
    constexpr BitBoard F5 = 0x0000002000000000;
    constexpr BitBoard G5 = 0x0000004000000000;
    constexpr BitBoard H5 = 0x0000008000000000;

    constexpr BitBoard A6 = 0x0000010000000000;
    constexpr BitBoard B6 = 0x0000020000000000;
    constexpr BitBoard C6 = 0x0000040000000000;
    constexpr BitBoard D6 = 0x0000080000000000;
    constexpr BitBoard E6 = 0x0000100000000000;
    constexpr BitBoard F6 = 0x0000200000000000;
    constexpr BitBoard G6 = 0x0000400000000000;
    constexpr BitBoard H6 = 0x0000800000000000;

    constexpr BitBoard A7 = 0x0001000000000000;
    constexpr BitBoard B7 = 0x0002000000000000;
    constexpr BitBoard C7 = 0x0004000000000000;
    constexpr BitBoard D7 = 0x0008000000000000;
    constexpr BitBoard E7 = 0x0010000000000000;
    constexpr BitBoard F7 = 0x0020000000000000;
    constexpr BitBoard G7 = 0x0040000000000000;
    constexpr BitBoard H7 = 0x0080000000000000;

    constexpr BitBoard A8 = 0x0100000000000000;
    constexpr BitBoard B8 = 0x0200000000000000;
    constexpr BitBoard C8 = 0x0400000000000000;
    constexpr BitBoard D8 = 0x0800000000000000;
    constexpr BitBoard E8 = 0x1000000000000000;
    constexpr BitBoard F8 = 0x2000000000000000;
    constexpr BitBoard G8 = 0x4000000000000000;
    constexpr BitBoard H8 = 0x8000000000000000;

    constexpr std::array<BitBoard, SquaresPerFile> RANKS = 
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

    constexpr std::array<BitBoard, SquaresPerRank> FILES = 
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

    constexpr BitBoard EDGES = A1|A2|A3|A4|A5|A6|A7|A8|B8|C8|D8|E8|F8|G8|H8|H7|H6|H5|H4|H3|H2|H1|G1|F1|E1|D1|C1|B1;
    constexpr BitBoard INNER = ~EDGES;
}


