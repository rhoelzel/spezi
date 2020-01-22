#pragma once

#include "BitBoardArray.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "Position.hpp"

#include <array>

namespace spezi
{
    auto constexpr MaxMoveNumber = 220; 
    
    // alles noch keine sehr elegante Lösung für MoveList
    auto constexpr FROM = 0;
    auto constexpr TO = 1;
    auto constexpr PIECE = 2;
    auto constexpr CAPTURED = 3;

    auto constexpr MoveSize = 4;

     // from, to, from piece, to piece
    using MoveList = std::array<Square, MaxMoveNumber * MoveSize>;
    using MoveAddress = Square *;


    template<Color color>
    MoveAddress advanceMoveListIfLegal(Position & position, MoveAddress nextMove)
    {
        auto const fromSquare = SQUARES[nextMove[FROM]];
        auto const toSquare = SQUARES[nextMove[TO]];
        auto constexpr other = (color + 1) % NumberOfColors;
        
        auto const backup1 = position.allPieces[color];
        auto const backup2 = position.allPieces[other];
        position.allPieces[color] ^= (fromSquare | toSquare);
        position.allPieces[other] &= ~toSquare;
        position.empty &= ~fromSquare;
        position.empty |= ~toSquare;

        auto king = ffs(position.allPieces[color] & position.individualPieces[KING]);
                 
        auto isIllegal = 
            (PawnPushesAttacks<color, true>[king] & position.allPieces[other] & position.individualPieces[PAWN]) 
            | (KnightAttacks[king] & position.allPieces[other] & position.individualPieces[KNIGHT])
            | (KingAttacks[king] & position.allPieces[other] & position.individualPieces[KING])
            | ((DiagonalAttacks[king][pext(~position.empty, DiagonalMasks[king])]
                & position.allPieces[other]) & (position.individualPieces[BISHOP] | position.individualPieces[QUEEN]))
            | (((RankAttacks[king][pext(~position.empty, RankMasks[king])]
                | FileAttacks[king][pext(~position.empty, FileMasks[king])])
                & position.allPieces[other]) & (position.individualPieces[ROOK] | position.individualPieces[QUEEN]));

        position.allPieces[color] = backup1;
        position.allPieces[other] = backup2;
        position.empty = ~(backup1 | backup2);

        if(!isIllegal)
        {
            nextMove += MoveSize;
        }

        return nextMove;
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

        while(movers)
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
                nextMove[CAPTURED] = KING;  // abuse the fact that kings are never captured
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                targets &= targets - 1;            
            }
            movers &= movers - 1;
        }

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

        while(movers)
        {
            auto const mover = ffs(movers);
            
            BitBoard targets {EMPTY};
            if constexpr(piece == ROOK || piece == QUEEN)
            {
                targets |= (RankAttacks[mover][pext(~position.empty, RankMasks[mover])]
                            | FileAttacks[mover][pext(~position.empty, FileMasks[mover])])
                            & position.empty;        
            }
            if constexpr(piece == BISHOP || piece == QUEEN)
            {
                targets |= DiagonalAttacks[mover][pext(~position.empty, DiagonalMasks[mover])] & position.empty;
            }
                      
            while(targets)
            {
                auto const target = ffs(targets);
                nextMove[FROM] = mover;
                nextMove[TO] = target;
                nextMove[PIECE] = piece;
                nextMove[CAPTURED] = KING;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                targets &= targets - 1;            
            }
            movers &= movers - 1;
        }
        
        return nextMove;
    }

    template<Piece piece, Color color>
    MoveAddress generateCaptureOf(Position & position, MoveAddress nextMove)
    {
        auto targets = position.allPieces[color] & position.individualPieces[piece];
        auto constexpr other = color % NumberOfColors;

        while(targets)
        {
            auto const target = ffs(targets);

            // pawns (least valuable attacker first)
            auto attackers = PawnPushesAttacks<color, true>[target] 
                                & position.allPieces[other]
                                & position.individualPieces[PAWN];        
                
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = PAWN;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                attackers &= attackers - 1;
            }

            // knights 
            attackers = KnightAttacks[target] 
                            & position.allPieces[other]
                            & position.individualPieces[KNIGHT];        
                
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = KNIGHT;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                attackers &= attackers - 1;
            }

            // bishops
            auto const diagonalEnemies = 
                DiagonalAttacks[target][pext(~position.empty, DiagonalMasks[target])]
                & position.allPieces[other];
            attackers = diagonalEnemies & position.individualPieces[BISHOP];
            
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = BISHOP;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                attackers &= attackers - 1;
            }

            // rooks
            auto const orthogonalEnemies = 
                (RankAttacks[target][pext(~position.empty, RankMasks[target])]
                | FileAttacks[target][pext(~position.empty, FileMasks[target])])
                & position.allPieces[other];
            attackers = orthogonalEnemies & position.individualPieces[ROOK];
            
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = ROOK;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                attackers &= attackers - 1;
            }

            // queens    
            attackers = (orthogonalEnemies | diagonalEnemies) & position.allPieces[other];
            
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = QUEEN;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                attackers &= attackers - 1;
            }

            // king 
            attackers = KingAttacks[target] 
                            & position.allPieces[other]
                            & position.individualPieces[KING];        
                
            if(attackers) // only one king
            {
                auto const attacker = ffs(targets);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = KING;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<color>(position, nextMove);
                attackers &= attackers - 1;
            }

            targets &= targets - 1;
        }
        
        return nextMove;
    }

    template<Color color>
    MoveList allMoves(Position & position)
    {
        MoveList result;

        auto nextMove = result.data();

        nextMove = generateRegularMovesBy<PAWN, WHITE, true>(position, nextMove);
        nextMove = generateRegularMovesBy<PAWN, WHITE, false>(position, nextMove);  
        nextMove = generateRegularMovesBy<KNIGHT, WHITE>(position, nextMove);
        nextMove = generateSlidingMovesBy<BISHOP, WHITE>(position, nextMove);
        nextMove = generateSlidingMovesBy<ROOK, WHITE>(position, nextMove);
        nextMove = generateSlidingMovesBy<QUEEN, WHITE>(position, nextMove);
        nextMove = generateRegularMovesBy<KING, WHITE>(position, nextMove);
        *nextMove = NULL_SQUARE;

        return result;
    }

    void prettyPrint(MoveList const & moveList);
}