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

        unmove<color, moveType>(position, nextMove);

        if(!isIllegal)
        {
                // probably needs a better place than here
            if constexpr(moveType == NON_CAPTURE || moveType == NON_CAPTURE_PROMOTION)
            {
                nextMove[CAPTURED] = NULL_PIECE;
            }
            else if constexpr(moveType == NON_CAPTURE || moveType == CAPTURE)
            {
                nextMove[PROMOTED] = NULL_PIECE;;
            }
            nextMove += MoveSize;
        }
        
        return nextMove;
    }

    template<Piece piece, Color color>
    MoveAddress generateNonCapturesBy(Position & position, MoveAddress nextMove)
    {
        auto movers = position.allPieces[color] & position.individualPieces[piece];
        auto constexpr promotionRank = ((color == WHITE) ? RANKS[SquaresPerFile-1] : RANKS[0]);

        while(movers)
        {
            auto const mover = ffs(movers);
                        
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
                        nextMove[PROMOTED] = promotedTo;
                        nextMove = advanceMoveListIfLegal<color, NON_CAPTURE_PROMOTION>(position, nextMove);
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
                nextMove = advanceMoveListIfLegal<color, NON_CAPTURE>(position, nextMove);
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
    MoveAddress generateCapturesOf(Position & position, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);
        
        BitBoard attackers;
        auto writeMoves = [&attackers, &nextMove, &position]
            (Square const target, Piece const attackingPiece)
        {
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                nextMove[FROM] = attacker;
                nextMove[TO] = target;
                nextMove[PIECE] = attackingPiece;
                nextMove[CAPTURED] = piece;
                nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
                attackers &= attackers - 1;
            }
        };

        auto targets = position.allPieces[color] & position.individualPieces[piece];

        while(targets)
        {
            auto const target = ffs(targets);

            // pawns (least valuable attacker first)
            attackers = PawnAttacks<color>[target] 
                            & position.allPieces[other]
                            & position.individualPieces[PAWN];        
                
            while(attackers)
            {
                auto const attacker = ffs(attackers);
                if(isPromotionSquare<other>(target))
                {
                    for(int promotedTo = static_cast<int>(QUEEN); promotedTo != static_cast<int>(PAWN); --promotedTo)
                    {
                        nextMove[FROM] = attacker;
                        nextMove[TO] = target;
                        nextMove[PROMOTED] = promotedTo;
                        nextMove[CAPTURED] = piece;
                        nextMove = advanceMoveListIfLegal<other, CAPTURE_PROMOTION>(position, nextMove);
                    }
                }
                else
                {
                    nextMove[FROM] = attacker;
                    nextMove[TO] = target;
                    nextMove[PIECE] = PAWN;
                    nextMove[CAPTURED] = piece;
                    nextMove = advanceMoveListIfLegal<other, CAPTURE>(position, nextMove);
                }
                attackers &= attackers - 1;
            }

            // knights 
            attackers = KnightAttacks[target] 
                            & position.allPieces[other]
                            & position.individualPieces[KNIGHT];        
            writeMoves(target, KNIGHT);     

            // bishops
            auto const diagonalEnemies = 
                DiagonalAttacks[target][pext(~position.empty, DiagonalMasks[target])]
                & position.allPieces[other];
            attackers = diagonalEnemies & position.individualPieces[BISHOP];
            writeMoves(target, BISHOP);     

            // rooks
            auto const orthogonalEnemies = 
                (RankAttacks[target][pext(~position.empty, RankMasks[target])]
                | FileAttacks[target][pext(~position.empty, FileMasks[target])])
                & position.allPieces[other];
            attackers = orthogonalEnemies & position.individualPieces[ROOK];
            writeMoves(target, ROOK);     
            
            // queens    
            attackers = (orthogonalEnemies | diagonalEnemies) & position.individualPieces[QUEEN];
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
    MoveAddress allMoves(Position & position, MoveAddress nextMove)
    {
        auto constexpr other = (color == WHITE ? BLACK : WHITE);

        nextMove = generateCapturesOf<QUEEN, other>(position, nextMove);
        nextMove = generateCapturesOf<ROOK, other>(position, nextMove);
        nextMove = generateCapturesOf<BISHOP, other>(position, nextMove);
        nextMove = generateCapturesOf<KNIGHT, other>(position, nextMove);
        nextMove = generateCapturesOf<PAWN, other>(position, nextMove);

        nextMove = generateNonCapturesBy<PAWN, color>(position, nextMove);  
        nextMove = generateNonCapturesBy<KNIGHT, color>(position, nextMove);
        nextMove = generateNonCapturesBy<BISHOP, color>(position, nextMove);
        nextMove = generateNonCapturesBy<ROOK, color>(position, nextMove);
        nextMove = generateNonCapturesBy<QUEEN, color>(position, nextMove);
        nextMove = generateNonCapturesBy<KING, color>(position, nextMove);
        
        *nextMove = NULL_SQUARE; nextMove[CAPTURED] = NULL_PIECE;
        return nextMove;
    }

    void prettyPrint(MoveAddress moveAddress);
}