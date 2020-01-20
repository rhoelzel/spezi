#pragma once

#include "BitBoard.hpp"
#include "Color.hpp"
#include "Square.hpp"

#include "BitBoardArrayDetail.hpp"

#include <array>

namespace spezi
{ 
    using BitBoardArray = std::array<BitBoard, NumberOfSquares>;
      
    BitBoardArray constexpr Ranks = detail::collectBitBoards(detail::rank);
    BitBoardArray constexpr Files = detail::collectBitBoards(detail::file);
    BitBoardArray constexpr RanksAndFiles = detail::collectBitBoards(detail::rankAndFile);
    BitBoardArray constexpr Diagonals = detail::collectBitBoards(detail::diagonals);

    BitBoardArray constexpr KingMoveAttacks = detail::collectBitBoards(detail::kingMoveAttack);

    auto constexpr RookOffsets = detail::collectOffsets(detail::rookMask, detail::rookSharingSquares);
    auto constexpr SharedRookAttacks = detail::collectSharedAttackArrays
        <RookOffsets[OFF_BOARD]>(detail::rookMoveAttack, detail::rookMask, RookOffsets);

    auto constexpr BishopOffsets = detail::collectOffsets(detail::bishopMask, detail::bishopSharingSquares);
    auto constexpr SharedBishopAttacks = detail::collectSharedAttackArrays
        <BishopOffsets[OFF_BOARD]>(detail::bishopMoveAttack, detail::bishopMask, BishopOffsets);
    
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
