#pragma once

#include "Color.hpp"
#include "Piece.hpp"
#include "Square.hpp"

#include <cstdint>

namespace spezi
{
    using ZKey = u_int64_t;
}

#include "ZKeyDetail.hpp"

namespace spezi
{
    auto constexpr BlackToMoveKey = detail::generateZKey();
    auto constexpr CastlingKeys = detail::generateZKeys<16>(BlackToMoveKey);
    auto constexpr EnPassantKeys = detail::generateZKeys<8>(CastlingKeys.back());
    auto constexpr PieceKeys = detail::pieceKeys(EnPassantKeys.back());
}