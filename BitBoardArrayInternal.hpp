#pragma once

#include "BitBoardArray.hpp"

namespace spezi::bba_int
{
    Square constexpr shiftSquare(Square const square, int8_t const rankShift, int8_t const fileShift)
    {
        if(square % SquaresPerRank + fileShift >= SquaresPerRank
            || square % SquaresPerRank + fileShift < 0
            || square / SquaresPerFile + rankShift >= SquaresPerFile
            || square / SquaresPerFile + rankShift < 0)
        {
            return -1;
        }
        return square + SquaresPerRank*rankShift + fileShift;            
    }
  
    BitBoard constexpr getBitBoardFromSquareChecked(Square const square)
    {
        if(square < 0 || square >= NumberOfSquares)
	{
	    return EMPTY;
	}
	return SQUARES[square];
    }
  
    BitBoard constexpr rank(Square const square)
    {
        auto const rank = square / SquaresPerFile;
        auto result = A1 << (rank * SquaresPerRank);    // square on A file of same rank
        result |= (result << 1);                        // square on B file
        result |= (result << 2);                        // squares on C,D files 
        result |= (result << 4);                        // squares on E,F,G,H files
        return result;
    }

    BitBoard constexpr file(Square const square)
    {
        auto const file = square % SquaresPerRank; 
        auto result = A1 << file;                       // square on 1st rank of same file    
        result |= (result << (1*SquaresPerRank));       // square on 2nd rank
        result |= (result << (2*SquaresPerRank));       // squares on 3rd and 4th ranks 
        result |= (result << (4*SquaresPerRank));       // squares on 5th through 8th ranks
        
        return result;
    }

    BitBoard constexpr rankAndFile(Square const square)
    {
        return rank(square) | file(square);            
    }

    BitBoard constexpr diagonals(Square const square)
    {
        auto result = EMPTY;
        for(int i = 1-SquaresPerFile; i < SquaresPerFile; ++i)
        {
	    if(square >= 0 && square < NumberOfSquares)
	    {
                result |= getBitBoardFromSquareChecked(shiftSquare(square, i, i));
                result |= getBitBoardFromSquareChecked(shiftSquare(square, -i, i));
            }
	}
        return result;    
    }
  
    BitBoard constexpr kingMoveAttack(Square const square)
    {
        auto result = getBitBoardFromSquareChecked(shiftSquare(square, -1, -1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, -1, 0));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, -1, 1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 0, -1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 0, 1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 1, -1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 1, 0));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 1, 1));  
    
        return result;
    }

    BitBoard constexpr rookMask(Square const square)
    {
        return ((rank(square) & ~FILES[0] & ~FILES[SquaresPerRank-1]) 
                | (file(square) & ~RANKS[0] & ~RANKS[SquaresPerFile-1]))
            & (~getBitBoardFromSquareChecked(square));
    }

    BitBoard constexpr bishopMask(Square const square)
    {
        return diagonals(square) & INNER & (~getBitBoardFromSquareChecked(square));    
    }

    BitBoard constexpr knightMoveAttack(Square const square)
    {
        auto result = getBitBoardFromSquareChecked(shiftSquare(square, -1, -2));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, -2, -1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, -2, 1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, -1, 2));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 1, 2));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 2, 1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 2, -1));  
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 1, -2));  
    
        return result;
    }

    BitBoard constexpr whitePawnMove(Square const square)
    {
        auto result = getBitBoardFromSquareChecked(shiftSquare(square, 1, 0));
        if(square / SquaresPerRank == 1)
        {
            result |= getBitBoardFromSquareChecked(shiftSquare(square, 2, 0));
        }
    
        return result & ~RANKS[1];
    }

    BitBoard constexpr whitePawnAttack(Square const square)
    {
        auto result = getBitBoardFromSquareChecked(shiftSquare(square, 1, -1));
        result |= getBitBoardFromSquareChecked(shiftSquare(square, 1, 1));
    
        return result & ~RANKS[1];
    }

    BitBoard constexpr blackPawnMove(Square const square)
    {
        auto result = getBitBoardFromSquareChecked(shiftSquare(square, -1, 0));
        if(square / SquaresPerRank == SquaresPerFile-2)
        {
            result |= getBitBoardFromSquareChecked(shiftSquare(square, -2, 0));
        }
    
        return result & ~RANKS[SquaresPerFile-2];
    }

    BitBoard constexpr blackPawnAttack(Square const square)
    {
        auto result = getBitBoardFromSquareChecked(shiftSquare(square, -1, -1));
        result |= getBitBoardFromSquareChecked(shiftSquare(square, -1, 1));
   
        return result & ~RANKS[SquaresPerFile-2];
    }
  
    auto constexpr AllSquares = std::make_integer_sequence<Square, NumberOfSquares>{};
    auto constexpr UpperRight4x4 = std::integer_sequence
    < Square,
        e5, f5, g5, h5, 
        e6, f6, g6, h6,
        e7, f6, g7, h7,
        e8, f8, g8, h8          
    > {};
  
    template <BitBoard bitBoardGenerator(Square const), Square... squares>
    std::array<BitBoard, sizeof...(squares)> constexpr collectBitBoards(std::integer_sequence<Square, squares...>)
    {
        return std::array<BitBoard, sizeof...(squares)> {bitBoardGenerator(squares)...};            
    }

  /*    constexpr rookOccupancy(Square const square, int const permutation)
    {
        auto const numberOfRelevantSquares = getNumberOfSetBitsInBitBoard(SQUARES[square]);
	
	}*/
}
