#pragma once

#include <array>
#include <utility>
#include <cstdint>

namespace spezi
{
    using Square = int8_t;
    
    Square constexpr SquaresPerRank = 8;
    Square constexpr SquaresPerFile = 8;
    Square constexpr NumberOfSquares = SquaresPerRank * SquaresPerFile;

    // note: lower case (like here) => Square index, upper case (like A1) => BitBoard
    Square constexpr a1 = 0x00;
    Square constexpr a2 = 0x01;
    Square constexpr a3 = 0x02;
    Square constexpr a4 = 0x03;
    Square constexpr a5 = 0x04;
    Square constexpr a6 = 0x05;
    Square constexpr a7 = 0x06;
    Square constexpr a8 = 0x07;
    Square constexpr b1 = 0x08;
    Square constexpr b2 = 0x09;
    Square constexpr b3 = 0x0A;
    Square constexpr b4 = 0x0B;
    Square constexpr b5 = 0x0C;
    Square constexpr b6 = 0x0D;
    Square constexpr b7 = 0x0E;
    Square constexpr b8 = 0x0F;
    Square constexpr c1 = 0x10;
    Square constexpr c2 = 0x11;
    Square constexpr c3 = 0x12;
    Square constexpr c4 = 0x13;
    Square constexpr c5 = 0x14;
    Square constexpr c6 = 0x15;
    Square constexpr c7 = 0x16;
    Square constexpr c8 = 0x17;
    Square constexpr d1 = 0x18;
    Square constexpr d2 = 0x19;
    Square constexpr d3 = 0x1A;
    Square constexpr d4 = 0x1B;
    Square constexpr d5 = 0x1C;
    Square constexpr d6 = 0x1D;
    Square constexpr d7 = 0x1E;
    Square constexpr d8 = 0x1F;
    Square constexpr e1 = 0x20;
    Square constexpr e2 = 0x21;
    Square constexpr e3 = 0x22;
    Square constexpr e4 = 0x23;
    Square constexpr e5 = 0x24;
    Square constexpr e6 = 0x25;
    Square constexpr e7 = 0x26;
    Square constexpr e8 = 0x27;
    Square constexpr f1 = 0x28;
    Square constexpr f2 = 0x29;
    Square constexpr f3 = 0x2A;
    Square constexpr f4 = 0x2B;
    Square constexpr f5 = 0x2C;
    Square constexpr f6 = 0x2D;
    Square constexpr f7 = 0x2E;
    Square constexpr f8 = 0x2F;
    Square constexpr g1 = 0x30;
    Square constexpr g2 = 0x31;
    Square constexpr g3 = 0x32;
    Square constexpr g4 = 0x33;
    Square constexpr g5 = 0x34;
    Square constexpr g6 = 0x35;
    Square constexpr g7 = 0x36;
    Square constexpr g8 = 0x37;
    Square constexpr h1 = 0x38;
    Square constexpr h2 = 0x39;
    Square constexpr h3 = 0x3A;
    Square constexpr h4 = 0x3B;
    Square constexpr h5 = 0x3C;
    Square constexpr h6 = 0x3D;
    Square constexpr h7 = 0x3E;
    Square constexpr h8 = 0x3F;
    
    Square constexpr OFF_BOARD = 0x40;
    Square constexpr NULL_SQUARE = -0x01;

    namespace Neighborhood
    {
        enum Direction
        {
            N = 0,
            NNE = 1,
            NE = 2,
            ENE = 3, 
            E = 4, 
            ESE = 5,
            SE = 6,
            SSE = 7,
            S = 8,
            SSW = 9,
            SW = 10,
            WSW = 11,
            W = 12,
            WNW = 13,
            NW = 14, 
            NNW = 15
        };

        int constexpr NumberOfDirections = 16;

        Direction constexpr KingQueenReachable[] = { N, NE, E, SE, S, SW, W, NW };
        Direction constexpr RookReachable[] = { N, E, S, W };
        Direction constexpr BishopReachable[] = { NE, SE, SW, NW };
        Direction constexpr KnightReachable[] = { NNE, ENE, ESE, SSE, SSW, WSW, WNW, NNW };
    };
}

#include "SquareDetail.hpp"

namespace spezi
{
    using NeighborArray = std::array<std::array<Square, Neighborhood::NumberOfDirections>, NumberOfSquares + 1>;
    
    NeighborArray constexpr Neighbors = detail::collectNeighbors();
}
