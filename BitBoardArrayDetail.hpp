#pragma once

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
      
    BitBoard constexpr kingMoveAttack(Square const square)
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

    BitBoard constexpr rookMask(Square const square)
    {
        return ((rank(square) & ~FILES[0] & ~FILES[SquaresPerRank-1]) 
                | (file(square) & ~RANKS[0] & ~RANKS[SquaresPerFile-1]))
                & (~SQUARES[square]);
    }
  
    BitBoard constexpr rookOccupancy(Square const square, BitBoard const permutation)
    {
        return pdep_slow(permutation, rookMask(square));
    }

    BitBoard constexpr rookMoveAttack(Square const square, BitBoard const permutation)
    {
        auto result = EMPTY;
	    for(auto const direction : RookReachable)
	    {
            for(auto s = Neighbors[square][direction]; s != OFF_BOARD; s = Neighbors[s][direction])
            {
	        auto const next = SQUARES[s];
                result |= next;
     
                if (next & rookOccupancy(square, permutation))
                {
                    break; // first blocker found
                }
            }
	    }
        return result;
    }

    BitBoard constexpr bishopMask(Square const square)
    {
        return diagonals(square) & INNER & (~SQUARES[square]);    
    }
  
    BitBoard constexpr bishopOccupancy(Square const square, BitBoard const permutation)
    {
        return pdep_slow(permutation, bishopMask(square));
    }

    BitBoard constexpr bishopMoveAttack(Square const square, BitBoard const permutation)
    {
        auto result = EMPTY;
	    for(auto const direction : BishopReachable)
	    {
            for(auto s = Neighbors[square][direction]; s != OFF_BOARD; s = Neighbors[s][direction])
            {
	            auto const next = SQUARES[s];
                result |= next;
     
                if (next & bishopOccupancy(square, permutation))
                {
                    break; // first blocker found
                }
            }
	    }
        return result;
    } 
     
    BitBoard constexpr knightMoveAttack(Square const square)
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

    // from here on, collect BitBoards of individual squares into arrays

    auto constexpr collectBitBoards(BitBoard bitBoardGenerator(Square const))
    {
        auto result = std::array<BitBoard, NumberOfSquares>{};
        for(Square s = 0;s<NumberOfSquares;++s)
        {
	        result[s] = bitBoardGenerator(s);
        }
        return result;
    }

    int constexpr rookSharingIndex[NumberOfSquares] = 
    {
        0,  1,  2,  3,  4,  5,  6,  7,
        1,  0,  3,  2,  5,  4,  7,  6,
        8,  9, 10, 11, 12, 13, 14, 15,
        9,  8, 11, 10, 13, 12, 15, 14,
       16, 17, 18, 19, 20, 21, 22, 23,
       17, 16, 19, 18, 21, 20, 23, 22,
       24, 25, 26, 27, 28, 29, 30, 31,
       25, 24, 27, 26, 29, 28, 31, 30
    };
    int constexpr rookSharingFactor = 2;

    int constexpr bishopSharingIndex[NumberOfSquares] = 
    {
        0,  2,  4,  4,  4,  4, 12, 14,
        0,  2,  5,  5,  5,  5, 12, 14,
        0,  2,  6,  6,  6,  6, 12, 14,
        0,  2,  7,  7,  7,  7, 12, 14,
        1,  3,  8,  8,  8,  8, 13, 15,
        1,  3,  9,  9,  9,  9, 13, 15,
        1,  3, 10, 10, 10, 10, 13, 15,
        1,  3, 11, 11, 11, 11, 13, 15
    };
    int constexpr bishopSharingFactor = 4;

   /* Bitboard constexpr condenseRookAttack(Square const square, BitBoard const permutation)
    {
        auto result = std::array<BitBoard, NumberOfSquares / rookSharingFactor>{};
        for(Square s = 0; s < NumberOfSquares; ++s)
        {
            result[rookSharingIndex[square]] |= 
        }
    }*/
}
