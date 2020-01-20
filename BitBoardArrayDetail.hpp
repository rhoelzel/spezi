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

    auto constexpr collectBitBoards(BitBoard bitBoardGenerator(Square))
    {
        auto result = std::array<BitBoard, NumberOfSquares>{};
        for(Square s = 0;s<NumberOfSquares;++s)
        {
	        result[s] = bitBoardGenerator(s);
        }
        return result;
    }

    Square constexpr rookSharingSquares[NumberOfSquares + 1] = 
    {
        a1, b1, c1, d1, e1, f1, g1, h1,
        b1, a1, d1, c1, f1, e1, h1, g1,
        a3, b3, c3, d3, e3, f3, g3, h3,
        b3, a3, d3, c3, f3, e3, h3, g3,
        a5, b5, c5, d5, e5, f5, g5, h5,
        b5, a5, d5, c5, f5, e5, h5, g5,
        a7, b7, c7, d7, e7, f7, g7, h7,
        b7, a7, d7, c7, f7, e7, h7, g7,
        OFF_BOARD, // loop end moniker
    };
    
    Square constexpr bishopSharingSquares[NumberOfSquares + 1] = 
    {
        a1, b1, c1, c1, c1, c1, g1, h1,
        a1, b1, c2, c2, c2, c2, g1, h1,
        a1, b1, c3, c3, c3, c3, g1, h1,
        a1, b1, c4, c4, c4, c4, g1, h1,
        a5, b5, c5, c5, c5, c5, g5, h5,
        a5, b5, c6, c6, c6, c6, g5, h5,
        a5, b5, c7, c7, c7, c7, g5, h5,
        a5, b5, c8, c8, c8, c8, g5, h5,
        OFF_BOARD, // loop end moniker
    };
    
    auto constexpr collectOffsets(BitBoard maskGenerator(Square), Square const * const sharingSquares)
    {
        auto offset = 0;
        
        std::array<BitBoard, NumberOfSquares + 1> result {};

    	for(Square square = 0; square != OFF_BOARD; ++square)
	    {
            if(square == sharingSquares[square])
            {
                result[square] = offset;
                offset += SQUARES[popcount(maskGenerator(square))];
            }
            else
            {
                result[square] = result[sharingSquares[square]];
            }
	    }
	    
    	result[OFF_BOARD] = offset;
	    return result;
    }

    template<size_t N>
    auto constexpr collectSharedAttackArrays(BitBoard attackGenerator(Square, BitBoard),
        BitBoard maskGenerator(Square), std::array<BitBoard, NumberOfSquares + 1> const & offsets)
    {
        std::array<BitBoard, N> result {};
        for(Square square = 0; square !=OFF_BOARD; ++square)
        {
            auto const numberOfPermutations = popcount(maskGenerator(square));
            for(BitBoard permutation = 0; permutation != numberOfPermutations; ++permutation)
            {
                result[offsets[square] + permutation] |= attackGenerator(square, permutation);
            }
        } 
        return result;
    }
}
