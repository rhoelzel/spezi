#pragma once

namespace spezi::detail
{
    using namespace Neighborhood;
  
    BitBoard constexpr rank(Square const square)
    {
        return square == NO_SQUARE ? EMPTY : RANKS[square / SquaresPerFile];
    }

    BitBoard constexpr file(Square const square)
    {
        return square == NO_SQUARE ? EMPTY : FILES[square % SquaresPerRank];
    }

    BitBoard constexpr rankAndFile(Square const square)
    {
        return rank(square) | file(square);            
    }

    BitBoard constexpr ray(Square const square, Direction const direction, BitBoard const previous = EMPTY)
    {
        if(square == NO_SQUARE)
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
            for(auto s = Neighbors[square][direction]; s != NO_SQUARE; s = Neighbors[s][direction])
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
            for(auto s = Neighbors[square][direction]; s != NO_SQUARE; s = Neighbors[s][direction])
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
        auto const singleStep = Neighbors[square][N];
	auto result = SQUARES[singleStep];
        if(square / SquaresPerRank == 1)
        {
	    result |= SQUARES[Neighbors[singleStep][N]];
        }
        return result & ~RANKS[1];
    }

    BitBoard constexpr whitePawnAttack(Square const square)
    {
        auto result = SQUARES[Neighbors[square][NW]]
	    | SQUARES[Neighbors[square][NE]];
        return result & ~RANKS[1];
    }

    BitBoard constexpr blackPawnMove(Square const square)
    {
        auto const singleStep = Neighbors[square][S];
	auto result = SQUARES[singleStep];
        if(square / SquaresPerRank == SquaresPerFile-2)
        {
	    result |= SQUARES[Neighbors[singleStep][S]];
        }
        return result & ~RANKS[SquaresPerFile-2];
    }

    BitBoard constexpr blackPawnAttack(Square const square)
    {
        auto result = SQUARES[Neighbors[square][SW]]
	    | SQUARES[Neighbors[square][SE]];
        return result & ~RANKS[SquaresPerFile-2];
    }

    auto constexpr collectBitBoards(BitBoard bitBoardGenerator(Square const))
    {
        auto result = std::array<BitBoard, NumberOfSquares+1>{};
        for(Square s = 0;s<NumberOfSquares;++s)
        {
	    result[s] = bitBoardGenerator(s);
        }
        return result;
    }

    auto constexpr collectPermutations(BitBoard maskGenerator(Square const))
    {
        auto result = std::array<BitBoard, NumberOfSquares+1>{};
        for(Square s = 0;s<NumberOfSquares;++s)
        {
	  result[s] = SQUARES[popcount(maskGenerator(s))];
        }
        return result;
    }

    auto constexpr hihi = collectPermutations(rookMask);

  auto constexpr cache()
  {
    auto result = 0;
    for(Square s = 0;s<NumberOfSquares;++s)
      {
	result += hihi[s];
      }
    return result;
  }

  
}
