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
        auto constexpr other = (color + 1) % NumberOfColors;
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

    template<Piece piece, Color color, bool doubleStep = false>
    MoveAddress generateRegularMovesBy(Position & position, MoveAddress nextMove)
    {
        static_assert(
            piece != BISHOP && 
            piece != ROOK && 
            piece != QUEEN, 
            "Use generateSlidingMoves(...)");
        
        static_assert((piece == PAWN) | !doubleStep, "What do you mean, 'doubleStep = true'?");

        auto movers = position.allPieces[color] & position.individualPieces[piece];
        auto constexpr other = (color + 1) % NumberOfColors;

        if(!movers)
        {
            return nextMove;
        }

        do
        {
            auto const mover = ffs(movers);
                        
            BitBoard targets;
            if constexpr(piece == PAWN)
            {
                targets = PawnPushesAttacks<color, false, doubleStep>[mover] & position.empty;        
            }
            else if constexpr(piece == KNIGHT)
            {
                targets = KnightAttacks[mover] & position.empty;
            }
            else
            {
                targets = KingAttacks[mover] & position.empty;
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

    template<Piece piece, Color color>
    MoveAddress generateSlidingMovesBy(Position & position, MoveAddress nextMove)
    {
        static_assert(
            piece == BISHOP || 
            piece == ROOK ||
            piece == QUEEN, 
            "Use generateRegularMoves(...)");
        
        auto movers = position.allPieces[color] & position.individualPieces[piece];

        if(!movers)
        {
            return nextMove;
        }

        do
        {
            auto const mover = ffs(movers);
            
            BitBoard targets {EMPTY};
            if constexpr(piece == ROOK || piece == QUEEN)
            {
                targets |= (RankAttacks[mover][pext(~position.empty, Ranks[mover])]
                            | FileAttacks[mover][pext(~position.empty, Files[mover])])
                            & position.empty;        
            }
            if constexpr(piece == BISHOP || piece == QUEEN)
            {
                targets |= DiagonalAttacks[mover][pext(~position.empty, Diagonals[mover])] & position.empty;
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

    MoveList allMoves(Position const & position, Color toMove);
}