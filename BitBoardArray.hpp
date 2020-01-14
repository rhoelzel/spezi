#pragma once

#include "BitBoard.hpp"
#include "Square.hpp"

#include "BitBoardArrayDetail.hpp"

#include <array>

namespace spezi
{ 
    using BitBoardArray = std::array<BitBoard, NumberOfSquares + 1>;
  
    BitBoardArray constexpr Ranks = detail::collectBitBoards(detail::rank);
    BitBoardArray constexpr Files = detail::collectBitBoards(detail::file);
    BitBoardArray constexpr RanksAndFiles = detail::collectBitBoards(detail::rankAndFile);
    BitBoardArray constexpr Diagonals = detail::collectBitBoards(detail::diagonals);
    BitBoardArray constexpr KingMoveAttacks = detail::collectBitBoards(detail::kingMoveAttack);
    BitBoardArray constexpr KnightMoveAttacks = detail::collectBitBoards(detail::knightMoveAttack);
    BitBoardArray constexpr WhitePawnMoves = detail::collectBitBoards(detail::whitePawnMove);
    BitBoardArray constexpr WhitePawnAttacks = detail::collectBitBoards(detail::whitePawnAttack);
    BitBoardArray constexpr BlackPawnMoves = detail::collectBitBoards(detail::blackPawnMove);
    BitBoardArray constexpr BlackPawnAttacks = detail::collectBitBoards(detail::blackPawnAttack);
}
