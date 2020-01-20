#pragma once

#include "BitBoard.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "Position.hpp"

#include <array>

namespace spezi
{
    auto constexpr MaxMoveNumber = 220; 
    
    // from, to, piece type
    using MoveList = std::array<Square, MaxMoveNumber * 3>;
    using MoveAddress = Square *;

    MoveList allMoves(Position const & position, Color toMove);
}