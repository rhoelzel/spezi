#pragma once

#include "BitBoardArray.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "Position.hpp"

#include <array>

namespace spezi
{
    // alles noch keine sehr elegante Lösung für die Move-Datenstrukturen
    auto constexpr FROM = 0;
    auto constexpr TO = 1;
    auto constexpr PIECE = 2;
    auto constexpr CAPTURED = 3;

    auto constexpr MoveSize = 4;

     // from, to, from piece, to piece
    using MoveAddress = Square *;

    enum MoveType
    {
        NON_CAPTURE,
        CAPTURE
    };

    template<Color color, MoveType moveType>
    void constexpr move(Position & position, MoveAddress const move)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);

        auto const from = A1 << move[FROM];
        auto const to = A1 << move[TO];
        auto const fromTo = from ^ to;

        position.allPieces[color] ^= fromTo;    
        position.individualPieces[move[PIECE]] ^= fromTo;           
        
        if constexpr(moveType == NON_CAPTURE)
        {
            position.empty ^= fromTo;    
        }
        else
        {
            position.allPieces[other] ^= to;         
            position.individualPieces[move[CAPTURED]] ^= to;
            position.empty ^= from;             
        }
    }

    template<Color color, MoveType moveType>
    auto constexpr unmove = move<color, moveType>;    

    template<Color color, MoveType moveType>
    MoveAddress advanceMoveListIfLegal(Position & position, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        
        move<color, moveType>(position, nextMove); 

        auto const king = ffs(position.allPieces[color] & position.individualPieces[KING]);
        auto isIllegal = 
            (PawnAttacks<color>[king] & position.allPieces[other] & position.individualPieces[PAWN]) 
            | (KnightAttacks[king] & position.allPieces[other] & position.individualPieces[KNIGHT])
            | (KingAttacks[king] & position.allPieces[other] & position.individualPieces[KING])
            | ((DiagonalAttacks[king][pext(~position.empty, DiagonalMasks[king])]
                & position.allPieces[other]) & (position.individualPieces[BISHOP] | position.individualPieces[QUEEN]))
            | (((RankAttacks[king][pext(~position.empty, RankMasks[king])]
                | FileAttacks[king][pext(~position.empty, FileMasks[king])])
                & position.allPieces[other]) & (position.individualPieces[ROOK] | position.individualPieces[QUEEN]));

        move<color, moveType>(position, nextMove);

        if(!isIllegal)
        {
            if constexpr(moveType == NON_CAPTURE)
            {
                // probably needs a better place than here
                nextMove[CAPTURED] = NULL_PIECE;
            }
            nextMove += MoveSize;
        }
        
        return nextMove;
    }

    template<Piece piece, Color color>
    MoveAddress generateRegularNonCapturesBy(Position & position, MoveAddress nextMove)
    {
        static_assert(
            piece != BISHOP && 
            piece != ROOK && 
            piece != QUEEN, 
            "Use generateSlidingNonCaptures(...)");
        
        auto movers = position.allPieces[color] & position.individualPieces[piece];

        while(movers)
        {
            auto const mover = ffs(movers);
                        
            BitBoard targets;
            if constexpr(piece == PAWN)
            {
                targets = PawnPushes<color>[mover] & position.empty;        
                // double pushes from starting position
                if constexpr(color == WHITE)
                {
                    targets |= (targets << SquaresPerRank) & position.empty & RANKS[3];
                }
                else
                {
                    targets |= (targets >> SquaresPerRank) & position.empty & RANKS[4];
                }
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
                nextMove = advanceMoveListIfLegal<color, NON_CAPTURE>(position, nextMove);
                targets &= targets - 1;            
            }
            movers &= movers - 1;
        }

        return nextMove;
    }

    template<Piece piece, Color color>
    MoveAddress generateSlidingNonCapturesBy(Position & position, MoveAddress nextMove)
    {
        static_assert(
            piece == BISHOP || 
            piece == ROOK ||
            piece == QUEEN, 
            "Use generateRegularNonCaptures(...)");
        
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
                nextMove = advanceMoveListIfLegal<color, NON_CAPTURE>(position, nextMove);
                targets &= targets - 1;            
            }
            movers &= movers - 1;
        }
        
        return nextMove;
    }
                
    template<Piece piece, Color color>
    MoveAddress generateCapturesOf(Position & position, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);

        auto targets = position.allPieces[color] & position.individualPieces[piece];

        while(targets)
        {
            auto const target = ffs(targets);

            // pawns (least valuable attacker first)
            auto attackers = PawnAttacks<color>[target] 
                                & position.allPieces[other]
                                & position.individualPieces[PAWN];        
                
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = PAWN;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
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
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
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
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
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
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
                attackers &= attackers - 1;
            }

            // queens    
            attackers = (orthogonalEnemies | diagonalEnemies) & position.individualPieces[QUEEN];
            
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = QUEEN;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
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
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
                attackers &= attackers - 1;
            }

            targets &= targets - 1;
        }
        
        return nextMove;
    }

    template<Color color>
    MoveAddress allMoves(Position & position, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);

        nextMove = generateCapturesOf<QUEEN, other>(position, nextMove);
        nextMove = generateCapturesOf<ROOK, other>(position, nextMove);
        nextMove = generateCapturesOf<BISHOP, other>(position, nextMove);
        nextMove = generateCapturesOf<KNIGHT, other>(position, nextMove);
        nextMove = generateCapturesOf<PAWN, other>(position, nextMove);

        nextMove = generateRegularNonCapturesBy<PAWN, color>(position, nextMove);  
        nextMove = generateRegularNonCapturesBy<KNIGHT, color>(position, nextMove);
        nextMove = generateSlidingNonCapturesBy<BISHOP, color>(position, nextMove);
        nextMove = generateSlidingNonCapturesBy<ROOK, color>(position, nextMove);
        nextMove = generateSlidingNonCapturesBy<QUEEN, color>(position, nextMove);
        nextMove = generateRegularNonCapturesBy<KING, color>(position, nextMove);
        
        *nextMove = NULL_SQUARE; nextMove[CAPTURED] = NULL_PIECE;
        return nextMove;
    }

    void prettyPrint(MoveAddress moveAddress);
}