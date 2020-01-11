#pragma once

#include "BitBoard.hpp"
#include "BitBoardArrayInternal.hpp"
#include "Square.hpp"

#include <array>
#include <utility>

namespace spezi
{  
    using BitBoardArray = std::array<BitBoard, NumberOfSquares>;
  
    BitBoardArray constexpr Ranks = bba_int::collectBitBoards<bba_int::rank>(bba_int::AllSquares);
    BitBoardArray constexpr Files = bba_int::collectBitBoards<bba_int::file>(bba_int::AllSquares);
    BitBoardArray constexpr RanksAndFiles = bba_int::collectBitBoards<bba_int::rankAndFile>(bba_int::AllSquares);
    BitBoardArray constexpr Diagonals = bba_int::collectBitBoards<bba_int::diagonals>(bba_int::AllSquares);
    BitBoardArray constexpr KingMoveAttacks = bba_int::collectBitBoards<bba_int::kingMoveAttack>(bba_int::AllSquares);
    BitBoardArray constexpr RookMasks = bba_int::collectBitBoards<bba_int::rookMask>(bba_int::AllSquares);
    BitBoardArray constexpr BishopMasks = bba_int::collectBitBoards<bba_int::bishopMask>(bba_int::AllSquares);
    BitBoardArray constexpr KnightMoveAttacks = bba_int::collectBitBoards<bba_int::knightMoveAttack>(bba_int::AllSquares);
    BitBoardArray constexpr WhitePawnMoves = bba_int::collectBitBoards<bba_int::whitePawnMove>(bba_int::AllSquares);
    BitBoardArray constexpr WhitePawnAttacks = bba_int::collectBitBoards<bba_int::whitePawnAttack>(bba_int::AllSquares);
    BitBoardArray constexpr BlackPawnMoves = bba_int::collectBitBoards<bba_int::blackPawnMove>(bba_int::AllSquares); 
    BitBoardArray constexpr BlackPawnAttacks = bba_int::collectBitBoards<bba_int::blackPawnAttack>(bba_int::AllSquares);
  
}
