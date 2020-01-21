#pragma once

#include <algorithm>

namespace spezi::detail
{
    using namespace Neighborhood;
  
    BitBoard constexpr rank(Square const square)
    {
        return RANKS[square / SquaresPerFile];
    }

    BitBoard constexpr file(Square const square)
    {
        return FILES[square % SquaresPerRank];
    }

    BitBoard constexpr rankAndFile(Square const square)
    {
        return rank(square) | file(square);            
    }

    BitBoard constexpr ray(Square const square, Direction const direction, BitBoard const previous = EMPTY)
    {
        if(square == OFF_BOARD)
        {
	        return previous;
	    }	
	    return ray(Neighbors[square][direction], direction, previous | SQUARES[square]); 
    }
  
    BitBoard constexpr diagonals(Square const square)
    {
        auto result = SQUARES[square];
	    for(auto const direction : BishopReachable)
	    {
	        result |= ray(Neighbors[square][direction], direction); 
	    }
	    return result;    
    }
      
    BitBoard constexpr kingAttack(Square const square)
    {
        auto result = EMPTY;
	    for(auto const direction : KingQueenReachable)
	    {
	        result |= SQUARES[Neighbors[square][direction]]; 
	    }
	    return result;    
    }

    // cannot use __asm__ in constexpr, but static computation does not need to be fast
    BitBoard constexpr pdep_slow(BitBoard const source, BitBoard mask)
    {
        BitBoard result = 0;
        int i = 0;

        while (mask) 
        {
            if (source & SQUARES[i++])
            result |= mask & -mask;
            mask &= mask - 1;
        }
        
        return result;
    }

    BitBoard constexpr rankMask(Square const square)
    {               
        return (rank(square) & ~FILES[0] & ~FILES[SquaresPerRank-1]) & ~SQUARES[square];
    }

    BitBoard constexpr fileMask(Square const square)
    {
        return (file(square) & ~RANKS[0] & ~RANKS[SquaresPerFile-1]) & (~SQUARES[square]);
    }
  
    BitBoard constexpr diagonalMask(Square const square)
    {
        return diagonals(square) & INNER & (~SQUARES[square]);    
    }

    BitBoard constexpr maskOccupancy(BitBoard maskGenerator(Square), 
        Square const square, BitBoard const permutation)
    {
        return pdep_slow(permutation, maskGenerator(square));
    }

    template<Neighborhood::Direction direction>
    BitBoard constexpr slidingAttack(BitBoard maskGenerator(Square),
        Square const square, BitBoard const permutation)
    {
        auto result = EMPTY;
	    
        for(auto s = Neighbors[square][direction]; s != OFF_BOARD; s = Neighbors[s][direction])
        {
	        auto const next = SQUARES[s];
            result |= next;
     
            if (next & maskOccupancy(maskGenerator, square, permutation))
            {
                break; // first blocker found
            }
	    }

        return result;
    } 

    BitBoard constexpr rankAttack(Square const square, BitBoard const permutation)
    {
        return slidingAttack<E>(rankMask, square, permutation)
                | slidingAttack<W>(rankMask, square, permutation);
    }

    BitBoard constexpr fileAttack(Square const square, BitBoard const permutation)
    {
        return slidingAttack<N>(fileMask, square, permutation)
                | slidingAttack<S>(fileMask, square, permutation);
    }

    BitBoard constexpr diagonalAttack(Square const square, BitBoard const permutation)
    {
        return slidingAttack<NE>(diagonalMask, square, permutation)
                | slidingAttack<SE>(diagonalMask, square, permutation)
                | slidingAttack<SW>(diagonalMask, square, permutation)
                | slidingAttack<NW>(diagonalMask, square, permutation);
    }

    BitBoard constexpr knightAttack(Square const square)
    {
        auto result = EMPTY;
	    for(auto const direction : KnightReachable)
	    {   
	        result |= SQUARES[Neighbors[square][direction]]; 
	    }
	    return result;    
    }

    BitBoard constexpr whitePawnMove(Square const square)
    {
        auto result = SQUARES[Neighbors[square][N]];
        return result & ~RANKS[1];
    }

    BitBoard constexpr whitePawnDoubleMove(Square const square)
    {
        auto result = SQUARES[Neighbors[Neighbors[square][N]][N]];
        return result & RANKS[3];
    }

    BitBoard constexpr whitePawnAttack(Square const square)
    {
        auto result = SQUARES[Neighbors[square][NW]] | SQUARES[Neighbors[square][NE]];
        return result & ~RANKS[1];
    }

    BitBoard constexpr blackPawnMove(Square const square)
    {
        auto result = SQUARES[Neighbors[square][S]];
        return result & ~RANKS[SquaresPerFile-2];
    }

    BitBoard constexpr blackPawnDoubleMove(Square const square)
    {
        auto result = SQUARES[Neighbors[Neighbors[square][S]][S]];
        return result & RANKS[4];
    }
    
    BitBoard constexpr blackPawnAttack(Square const square)
    {
        auto result = SQUARES[Neighbors[square][SW]] | SQUARES[Neighbors[square][SE]];
        return result & ~RANKS[SquaresPerFile-2];
    }

    template<Color color, bool attack, bool doubleStep>
    BitBoard constexpr pawnPushAttack(Square const square)
    {
        static_assert(~(doubleStep & attack), "cannot have double step pawn attacks");
        if constexpr(color == WHITE)
        {
            if constexpr(attack)
            {
                return whitePawnAttack(square);
            }
            else
            {
                if constexpr(doubleStep)
                {
                    return whitePawnDoubleMove(square);
                }
                else
                {
                    return whitePawnMove(square);
                }
            }            
        }
        else
        {
            if constexpr(attack)
            {
                return blackPawnAttack(square);
            }
            else
            {
                if constexpr(doubleStep)
                {
                    return blackPawnDoubleMove(square);
                }
                else
                {
                    return blackPawnMove(square);
                }
            }
        }
    }

    // from here on, collect BitBoards of individual squares into arrays
    auto constexpr collectBitBoards(BitBoard bitBoardGenerator(Square))
    {
        auto result = std::array<BitBoard, NumberOfSquares>{};
        for(Square s = 0;s<NumberOfSquares;++s)
        {
	        result[s] = bitBoardGenerator(s);
        }
        return result;
    }

    template<BitBoard bitBoardGenerator(Square, BitBoard)>
    auto constexpr collectBitBoards()
    {
        // max permutations on d4 (9 for diagonals) or a1 (6 for ranks/files, need a corner)
        auto constexpr MaxNumberOfPermutations = 
            std::max
            (
                1 << (popcount(bitBoardGenerator(d4, 0)) - 4),  // ignore 4 edge squares
                1 << (popcount(bitBoardGenerator(a1, 0)) - 1)   // ignore 1 edge square
            );
        
        auto result = std::array<std::array<BitBoard, MaxNumberOfPermutations>, NumberOfSquares> {};

        for(Square s = 0;s < NumberOfSquares; ++s)
        {
            for(BitBoard p = 0; p < MaxNumberOfPermutations; ++p)
            {
	            result[s][p] = bitBoardGenerator(s,p);
            }
        }
        return result;
    }
}
