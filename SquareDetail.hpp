#pragma once

#include <cstdint>

namespace spezi::detail
{
    using namespace Neighborhood;
  
    Square constexpr shiftSquare(Square const square, int8_t const rankShift, int8_t const fileShift)
    {
        if(square % SquaresPerRank + fileShift >= SquaresPerRank
            || square % SquaresPerRank + fileShift < 0
            || square / SquaresPerFile + rankShift >= SquaresPerFile
            || square / SquaresPerFile + rankShift < 0
	    || square == OFF_BOARD)
        {
            return OFF_BOARD;
        }
        return square + SquaresPerRank*rankShift + fileShift;            
    }

    Square constexpr getNeighbor(Square const square, Direction const direction)
    {
        switch(direction)
        {
            case N:
                return shiftSquare(square, 1, 0);
            case NNE:
                return shiftSquare(square, 2, 1);
            case NE:
                return shiftSquare(square, 1, 1);
            case ENE:
                return shiftSquare(square, 1, 2);
            case E:
                return shiftSquare(square, 0, 1);
            case ESE:
                return shiftSquare(square, -1, 2);
            case SE:
                return shiftSquare(square, -1, 1);
            case SSE:
                return shiftSquare(square, -2, 1);
            case S:
                return shiftSquare(square, -1, 0);
            case SSW:
                return shiftSquare(square, -2, -1);
            case SW:
                return shiftSquare(square, -1, -1);
            case WSW:
                return shiftSquare(square, -1, -2);
            case W:
                return shiftSquare(square, 0, -1);
            case WNW:
                return shiftSquare(square, 1, -2);
            case NW:
                return shiftSquare(square, 1, -1);
            case NNW:
                return shiftSquare(square, 2, -1);
            default:
                return OFF_BOARD;
        }
    }

    auto constexpr collectNeighborsPerSquare(Square const square)
    {
        auto result = std::array<Square, NumberOfDirections>{};
        for(auto direction = 0; direction < NumberOfDirections; ++direction)
	    {
	        result[direction] = getNeighbor(square, static_cast<Direction>(direction));            
	    }
	    return result;
    }

    auto constexpr collectNeighbors()
    {
        auto result = std::array< std::array<Square, NumberOfDirections>, NumberOfSquares + 1>{};
        for(Square square = 0; square < NumberOfSquares + 1 ; ++square)
	    {
	        result[square] = collectNeighborsPerSquare(square);            
	    }
	    return result;
    }
}
