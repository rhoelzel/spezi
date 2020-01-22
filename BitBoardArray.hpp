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

    BitBoardArray constexpr KingAttacks = detail::collectBitBoards(detail::kingAttack);
    auto constexpr RankAttacks = detail::collectBitBoards<detail::rankAttack>();
    auto constexpr RankMasks = detail::collectBitBoards(detail::rankMask);
    auto constexpr FileAttacks = detail::collectBitBoards<detail::fileAttack>();
    auto constexpr FileMasks = detail::collectBitBoards(detail::fileMask);
    auto constexpr DiagonalAttacks = detail::collectBitBoards<detail::diagonalAttack>();    
    auto constexpr DiagonalMasks = detail::collectBitBoards(detail::diagonalMask);
    BitBoardArray constexpr KnightAttacks = detail::collectBitBoards(detail::knightAttack);
    template<Color color> 
    auto constexpr PawnAttacks = detail::collectBitBoards(detail::pawnAttack<color>);
    template<Color color> 
    auto constexpr PawnPushes = detail::collectBitBoards(detail::pawnPush<color>);
}
