#pragma once

#include "Color.hpp"
#include "Piece.hpp"
#include "Square.hpp"

#include "MobilityDetail.hpp"

#include <array>
#include <cstdint>

namespace spezi
{
    // Mobility is measured in MilliSquare units. One reachable square for any piece
    // equals 2^10 = 1024 MilliSquares => a Queen on d4 on an otherwise empty board
    // has a single move mobility of 27 * 1024 = 27648 MilliSquares
    using MilliSquare = int;
    Square constexpr milliToSquare(MilliSquare const milli) { return milli >> 10; }
    MilliSquare constexpr squareToMilli(Square const square) { return square << 10; }

    using MobilityDistribution = std::array<MilliSquare, detail::MaxBoardPopulation-2>;
    using MobilityArray = std::array<MobilityDistribution, NumberOfSquares>;

    // Static mobilities are computed as the average of mobilities for a given piece
    // on a random board populated with a given number of other pieces. Single move
    // mobility contributes fully to the static mobility, squares that are reachable 
    // on the second/third/fourth/... move contribute with weight 0.5/0.25/0.125/...
    // => a Queen on d4 has a static mobility of 37945 MilliSquares
    template<Color color, Piece piece>
    MobilityArray constexpr StaticMobilities = detail::averageMobilities<color, piece>(); 
    auto constexpr populationIndex(Square const population) { return population - 3; } 

    // Static mobility of a white pawn on d4 on a half full board (3180 MilliSquares) 
    MilliSquare constexpr PawnUnit = StaticMobilities<WHITE, PAWN>[d4][populationIndex(32)];

    float constexpr milliToPawnUnit(MilliSquare const mobility)
    {
        return static_cast<float>(mobility) / static_cast<float>(PawnUnit);
    }

    void prettyPrint(MobilityArray const & mobilityArray, Square const populatedSquares);
    void prettyPrint(MobilityDistribution const & mobilityDistribution);
}
