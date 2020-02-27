#pragma once

#include "BitBoardArray.hpp"
#include "Mobility.hpp"
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
    auto constexpr PROMOTED = 4;

    auto constexpr MoveSize = 5;

     // from, to, from piece, to piece
    using MoveAddress = Square *;

    enum MoveType
    {
        NON_CAPTURE,
        CAPTURE, 
        NON_CAPTURE_PROMOTION,
        CAPTURE_PROMOTION,
        CASTLING
    };

    template<Color color, MoveType moveType>
    void constexpr move(Position & position, MoveAddress const move)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);

        auto const from = A1 << move[FROM];
        auto const to = A1 << move[TO];
        auto const fromTo = from ^ to;

        position.allPieces[color] ^= fromTo;    
        
        if constexpr(moveType == NON_CAPTURE)
        {
            position.individualPieces[move[PIECE]] ^= fromTo;           
            position.empty ^= fromTo;    
        }
        else if constexpr(moveType == CAPTURE)
        {
            position.individualPieces[move[PIECE]] ^= fromTo;           
            position.allPieces[other] ^= to;         
            position.individualPieces[move[CAPTURED]] ^= to;
            position.empty ^= from;             
        }
        else if constexpr(moveType == NON_CAPTURE_PROMOTION)
        {
            position.individualPieces[PAWN] ^= from;
            position.individualPieces[move[PROMOTED]] ^= to;          
            position.empty ^=fromTo;
        }
        else if constexpr(moveType == CAPTURE_PROMOTION)
        {
            position.individualPieces[PAWN] ^= from;
            position.individualPieces[move[PROMOTED]] ^= to;          
            position.allPieces[other] ^= to;
            position.individualPieces[move[CAPTURED]] ^= to;
            position.empty ^= from;
        }
        else // castling
        {
        }
    }

    template<Color color, MoveType moveType>
    auto constexpr unmove = move<color, moveType>;    

    template<Piece piece, Color color>
    MoveAddress generateNonCapturesBy(Position & position, MilliSquare & evaluation, MoveAddress nextMove)
    {
        auto constexpr promotionRank = ((color == WHITE) ? RANKS[SquaresPerFile-1] : RANKS[0]);
        auto const population = popcount(~position.empty);

        auto movers = position.allPieces[color] & position.individualPieces[piece];

        while(movers)
        {
            auto const mover = ffs(movers);
            evaluation += StaticMobilities<piece, color>[mover][populationIndex(population)];

            BitBoard targets { EMPTY };

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

                // handle promotions before anything else
                auto lastRankTargets = targets & promotionRank;
                while(lastRankTargets)
                {
                    auto const target = ffs(lastRankTargets);
                    for(int promotedTo = static_cast<int>(QUEEN); promotedTo != static_cast<int>(PAWN); --promotedTo)
                    {
                        nextMove[FROM] = mover;
                        nextMove[TO] = target;
                        nextMove[PIECE] = piece;
                        nextMove[CAPTURED] = NULL_PIECE;
                        nextMove[PROMOTED] = promotedTo;
                        nextMove+=MoveSize;
                    }
                    lastRankTargets &= lastRankTargets - 1;
                }
                targets &= (~promotionRank);
            }
            else if constexpr(piece == KNIGHT)
            {
                targets = KnightAttacks[mover] & position.empty;
            }
            else if constexpr(piece == BISHOP || piece == QUEEN)
            {
                targets |= DiagonalAttacks[mover][pext(~position.empty, DiagonalMasks[mover])] & position.empty;
            }
            else if constexpr(piece == ROOK || piece == QUEEN)
            {
                targets |= (RankAttacks[mover][pext(~position.empty, RankMasks[mover])]
                            | FileAttacks[mover][pext(~position.empty, FileMasks[mover])])
                            & position.empty;        
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
                nextMove[CAPTURED] = NULL_PIECE;
                nextMove[PROMOTED] = NULL_PIECE;
                nextMove+=MoveSize;
                targets &= targets - 1;            
            }
            movers &= movers - 1;
        }

        return nextMove;
    }
                
    template<Color color>
    bool constexpr isPromotionSquare(Square const target)
    {
        if constexpr(color == WHITE)
        {
            return target > h7;
        }
        else
        {
            return target < h2;
        }
    }

    template<Piece piece, Color color>
    MoveAddress generateCapturesOf(Position & position, MilliSquare & evaluation, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        auto const population = popcount(~position.empty);

        BitBoard attackers;
        auto const writeMoves = [&attackers, &nextMove]
            (Square const target, Piece const attackingPiece)
        {
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = attackingPiece;
                nextMove[CAPTURED] = piece;
                nextMove[PROMOTED] = NULL_PIECE;
                nextMove+=MoveSize;
                attackers &= attackers - 1;
            }
        };

        auto targets = position.allPieces[color] & position.individualPieces[piece];

        while(targets)
        {
            auto const target = ffs(targets);
            evaluation -= StaticMobilities<piece, color>[target][populationIndex(population)];

            // pawns (least valuable attacker first)
            attackers = PawnAttacks<color>[target] 
                            & position.allPieces[other]
                            & position.individualPieces[PAWN];        
                
            while(attackers)
            {
                if constexpr(piece == KING) { return nullptr; }
                auto const attacker = ffs(attackers);
                if(isPromotionSquare<other>(target))
                {
                    for(int promotedTo = static_cast<int>(QUEEN); promotedTo != static_cast<int>(PAWN); --promotedTo)
                    {
                        nextMove[FROM] = attacker;
                        nextMove[TO] = target;
                        nextMove[PIECE] = PAWN;
                        nextMove[CAPTURED] = piece;
                        nextMove[PROMOTED] = promotedTo;
                        nextMove+=MoveSize;
                    }
                }
                else
                {
                    nextMove[FROM] = attacker;
                    nextMove[TO] = target;
                    nextMove[PIECE] = PAWN;
                    nextMove[CAPTURED] = piece;
                    nextMove[PROMOTED] = NULL_PIECE;
                    nextMove+=MoveSize;
                }
                attackers &= attackers - 1;
            }

            // knights 
            attackers = KnightAttacks[target] 
                            & position.allPieces[other]
                            & position.individualPieces[KNIGHT];      
            if constexpr(piece == KING) { if(attackers) { return nullptr; } }  
            writeMoves(target, KNIGHT);     

            // bishops
            auto const diagonalEnemies = 
                DiagonalAttacks[target][pext(~position.empty, DiagonalMasks[target])]
                & position.allPieces[other];
            attackers = diagonalEnemies & position.individualPieces[BISHOP];
            if constexpr(piece == KING) { if(attackers) { return nullptr; } }  
            writeMoves(target, BISHOP);     

            // rooks
            auto const orthogonalEnemies = 
                (RankAttacks[target][pext(~position.empty, RankMasks[target])]
                | FileAttacks[target][pext(~position.empty, FileMasks[target])])
                & position.allPieces[other];
            attackers = orthogonalEnemies & position.individualPieces[ROOK];
            if constexpr(piece == KING) { if(attackers) { return nullptr; } }  
            writeMoves(target, ROOK);     
            
            // queens    
            attackers = (orthogonalEnemies | diagonalEnemies) & position.individualPieces[QUEEN];
            if constexpr(piece == KING) { if(attackers) { return nullptr; } }  
            writeMoves(target, QUEEN);     
            
            // king 
            attackers = KingAttacks[target] 
                            & position.allPieces[other]
                            & position.individualPieces[KING];        
            writeMoves(target, KING);     
            
            targets &= targets - 1;
        }
        
        return nextMove;
    }

    template<Color color>
    MoveAddress allMoves(Position & position, MilliSquare & evaluation, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        evaluation = 0;

        if(generateCapturesOf<KING, other>(position, evaluation, nextMove) == nullptr)
        {
            evaluation = 1000 * PawnUnit;
            nextMove[CAPTURED] = NULL_PIECE;
            return nextMove;
        }

        nextMove = generateCapturesOf<QUEEN, other>(position, evaluation, nextMove);
        nextMove = generateCapturesOf<ROOK, other>(position, evaluation, nextMove);
        nextMove = generateCapturesOf<BISHOP, other>(position, evaluation, nextMove);
        nextMove = generateCapturesOf<KNIGHT, other>(position, evaluation, nextMove);
        nextMove = generateCapturesOf<PAWN, other>(position, evaluation, nextMove);

        nextMove = generateNonCapturesBy<PAWN, color>(position, evaluation, nextMove);  
        nextMove = generateNonCapturesBy<KNIGHT, color>(position, evaluation, nextMove);
        nextMove = generateNonCapturesBy<BISHOP, color>(position, evaluation, nextMove);
        nextMove = generateNonCapturesBy<ROOK, color>(position, evaluation, nextMove);
        nextMove = generateNonCapturesBy<QUEEN, color>(position, evaluation, nextMove);
        nextMove = generateNonCapturesBy<KING, color>(position, evaluation, nextMove);
        
        nextMove[CAPTURED] = NULL_PIECE;
        return nextMove;
    }

    void prettyPrint(MoveAddress firstMove, MoveAddress lastMove);
}