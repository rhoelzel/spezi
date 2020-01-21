#pragma once

#include "BitBoardArray.hpp"
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

    auto constexpr FROM = 0;
    auto constexpr TO = 1;
    auto constexpr PIECE = 2;

    template<Color color>
    int isLegal(Position & position, MoveAddress nextMove)
    {
        Square const king = ffs(position.allPieces[color] & position.individualPieces[KING]);
        auto constexpr other = color % NumberOfColors;
        BitBoard const from = a1 << nextMove[FROM];
        BitBoard const to = a1 << nextMove[TO];

        position.allPieces[color]^=from;
        position.allPieces[color]^=to;
        BitBoard const backupOther = position.allPieces[other];
        position.allPieces[other]&=~to;

        if((PawnPushesAttacks<color, true, false>[king] & position.allPieces[other] & position.individualPieces[PAWN])
            || (KnightAttacks[king] & position.allPieces[other] & position.individualPieces[KNIGHT]))
        {
                return 0;
        }
                
        return 1;
    }

    template<Piece piece, Color color, bool captures, bool doubleStep = false>
    MoveAddress generateRegularMoves(Position & position, MoveAddress nextMove)
    {
        static_assert(
            piece != BISHOP && 
            piece != ROOK && 
            piece != QUEEN, 
            "Use generateSlidingMoves(...)");
        
        static_assert((piece == PAWN) | !doubleStep, "What do you mean, 'doubleStep = true'?");

        auto movers = position.allPieces[color] & position.individualPieces[piece];
        
        if(!movers)
        {
            return nextMove;
        }

        BitBoard targetMask;
        if constexpr(captures)
        {
                targetMask = position.allPieces[color % NumberOfColors];
        }
        else
        {
            targetMask = ~(position.allPieces[color] | position.allPieces[color % NumberOfColors]);
        }

        do
        {
            auto const mover = ffs(movers);
                        
            BitBoard targets;
            if constexpr(piece == PAWN)
            {
                targets = PawnPushesAttacks<color, captures, doubleStep>[mover] & targetMask;        
            }
            else if constexpr(piece == KNIGHT)
            {
                targets = KnightAttacks[mover] & targetMask;
            }
            else
            {
                targets = KingAttacks[mover] & targetMask;
            }
            
            while(targets)
            {
                auto const target = ffs(targets);
                nextMove[FROM] = mover;
                nextMove[TO] = target;
                nextMove[PIECE] = piece;
                nextMove += isLegal<color>(position, nextMove);
                targets &= targets - 1;            
            }
            movers &= movers - 1;
        }
        while(movers);

        return nextMove;
    }
/*<
    template<Piece piece, Color color, bool captures>
    MoveAddress generateSlidingMoves(Position & position, MoveAddress nextMove)
    {
        static_assert(
            piece == BISHOP || 
            piece == ROOK ||
            piece == QUEEN, 
            "Use generateRegularMoves(...)");
        
        auto movers = position.allPieces[color] & position.individualPieces[piece];

        if(!movers)
        {
            return MoveAddress;
        }

        BitBoard targetMask;
        if constexpr(captures)
        {
            targetMask = position.allPieces[color % NumberOfColors];
        }
        else
        {
            targetMask = ~(position.allPieces[color] | position.allPieces[color % NumberOfColors]);
        }

        while(movers)
        {
            auto const mover = ffs(movers);
            
            BitBoard targets {EMPTY};
            if constexpr(piece == ROOK || piece == QUEEN)
            {
                targets =         
            }
            if constexpr(piece = KNIGHT)
            {
                targets = KingAttacks[mover] & targetMask;
            }
            else
            {
                targets = KnightAttacks[mover] & targetMask;
            }
            
            while(targets)
            {
                auto const target = ffs(targets);
                nextMove[FROM] = mover;
                nextMove[TO] = target;
                nextMove[PIECE] = piece;
                nextMove += isLegal<color>(position, nextMove);
                targetss &= targets - 1;            
            }
            movers &= movers - 1;
        }
        return nextMove;
    }*/

    MoveList allMoves(Position const & position, Color toMove);
}