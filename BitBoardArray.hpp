#pragma once

#include "BitBoard.hpp"
#include "Color.hpp"
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
    auto constexpr PawnMoves = std::array<BitBoardArray, NumberOfColors>
    {
        detail::collectBitBoards(detail::whitePawnMove),
        detail::collectBitBoards(detail::blackPawnMove)
    };
    auto constexpr PawnDoubleMoves = std::array<BitBoardArray, NumberOfColors>
    {
        detail::collectBitBoards(detail::whitePawnDoubleMove),
        detail::collectBitBoards(detail::blackPawnDoubleMove)
    };
    auto constexpr PawnAttacks = std::array<BitBoardArray, NumberOfColors>
    {
        detail::collectBitBoards(detail::whitePawnAttack),
        detail::collectBitBoards(detail::blackPawnAttack)
    };
}
