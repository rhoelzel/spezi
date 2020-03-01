#pragma once

#include "BitBoard.hpp"
#include "BitBoardArray.hpp"
#include "MobilityCoefficients.hpp"
#include <cmath>
#include <random>

namespace spezi::detail
{
    template<Piece piece>
    BitBoard constexpr reachable(Square const square, BitBoard const bb = EMPTY)
    {
        BitBoard result = EMPTY;

        if constexpr(piece == KING)
        {
            result = KingAttacks[square];
        }
        else if constexpr(piece == QUEEN)
        {
            result = (RankAttacks[square][pext(bb, RankMasks[square])]
                    | FileAttacks[square][pext(bb, FileMasks[square])]
                    | DiagonalAttacks[square][pext(bb,DiagonalMasks[square])]);
        }
        else if constexpr(piece == ROOK)
        {
            result = (RankAttacks[square][pext(bb, RankMasks[square])]
                    | FileAttacks[square][pext(bb, FileMasks[square])]);
        }
        else if constexpr(piece == BISHOP)
        {
            result = DiagonalAttacks[square][pext(bb, DiagonalMasks[square])];
        }
        else if constexpr(piece == KNIGHT)
        {
            result = KnightAttacks[square];
        }
        else if constexpr(piece == PAWN) 
        {
            // pawns be white here without loss of generality
            result = (PawnAttacks<WHITE>[square] & bb) | (PawnPushes<WHITE>[square] & ~bb);
            result |= (result << SquaresPerRank) & ~bb & RANKS[3] & FILES[square % SquaresPerRank];
        }
        else
        {
            result = EMPTY;
        }

        return result;
    }

    BitBoard random(Square const square, Square const population, 
        std::mt19937_64 & gen, std::uniform_int_distribution<Square> & dis);
    
    template<Piece piece> 
    double mobility(BitBoard const populatedBoard, 
        BitBoard & reachedUpToLastMove, BitBoard & reachedUpToThisMove)
    {
        auto promotion = EMPTY;

        auto result = 0.;
        auto allReached = EMPTY;
        auto init = reachedUpToThisMove & ~reachedUpToLastMove;

        if constexpr(piece == PAWN)
        {
            // for pawns only : treat promotions differently
            promotion |= init & RANKS[SquaresPerFile-1];
        }        
        while(init)
        {      
            auto const square = ffs(init);
            auto const reached = promotion ? 
                reachable<QUEEN>(square, populatedBoard) : reachable<piece>(square, populatedBoard);
            result += popcount(reached);
            allReached |= reached;
            init &= (init-1);
        }
        if(allReached & ~reachedUpToThisMove)
        {
            result /= popcount(reachedUpToThisMove & ~reachedUpToLastMove);
        }
        else
        {
            result = 0.;
        }        

        reachedUpToLastMove = reachedUpToThisMove;
        reachedUpToThisMove |= allReached;

        return result;
    }

    template<Piece piece>
    auto averageMobility(Square square, Square const population, double factor, 
        std::mt19937_64 & gen, std::uniform_int_distribution<Square> & dis, double const max = 500000.)
    {
        auto result = 0.;

        for(auto i = 0;i<max;++i)
        {
            auto const populatedBoard = random(square, population, gen, dis);
                  
            auto reachedUpToLastMove = EMPTY;
            auto reachedUpToThisMove = SQUARES[square];
            
            auto powerOfFactor = 1.0;

            while(reachedUpToThisMove & ~reachedUpToLastMove)
            {
                result += powerOfFactor *
                    mobility<piece>(populatedBoard, reachedUpToLastMove, reachedUpToThisMove);
                powerOfFactor *= factor;
            }
        }

        return result/max;
    }

    template<Piece piece>
    auto averageMobilities(Square const population)
    {
        auto result = std::array<double, NumberOfSquares>{};
        for(Square s = 0;s<NumberOfSquares;++s)
        {
	        result[s] = averageMobility<piece>(s, population);
        }
        return result;
    }

    // logic above is only used to generate MobilityCoefficients.hpp (via polynomial fitting)
    // from here on, actual mobilities used by the program are computed
    
    Square constexpr MaxBoardPopulation = 32;  

    auto constexpr averageMobility(Piece const piece, Square const square, Square const population)
    {
        if(!(population <= MaxBoardPopulation && population > 2))
        {
            throw std::runtime_error("must have population of at least 3 (kings + piece) and at most 32 total pieces");
        }

        if(piece == PAWN)
        {
            if(square < a2 || square > h7)
            {
                return 0.0;
            }       
        }

        // map 32 to 0, 3 to 29
        auto const index = MaxBoardPopulation - population;
 
        double const x = index;
        auto const x2 = x*x;
        auto const x3 = x2*x;
        auto const x4 = x2*x2;
        
        auto const a = MobilityCoefficients[(square * NumberOfPieceTypes + piece) * NumberOfPolyCoeffs + 0];
        auto const b = MobilityCoefficients[(square * NumberOfPieceTypes + piece) * NumberOfPolyCoeffs + 1];
        auto const c = MobilityCoefficients[(square * NumberOfPieceTypes + piece) * NumberOfPolyCoeffs + 2];
        auto const d = MobilityCoefficients[(square * NumberOfPieceTypes + piece) * NumberOfPolyCoeffs + 3];
        auto const e = MobilityCoefficients[(square * NumberOfPieceTypes + piece) * NumberOfPolyCoeffs + 4];
                
        return a*x4+b*x3+c*x2+d*x+e;
    }

    template<Color color, Piece piece>
    auto constexpr averageMobilities()
    {      
        std::array<std::array<int, MaxBoardPopulation-2>, NumberOfSquares> result {};
        for(auto square = a1; square != OFF_BOARD; ++square)
        {
            auto const flippedSquare = (color == WHITE ? square : NumberOfSquares-square-1);
            for(auto population = 3; population <= MaxBoardPopulation; ++population)
            {
                result[square][population-3] = static_cast<int>(averageMobility(piece, flippedSquare, population) * 1024 + 0.5);
            }
        }
        return result;
    }
}
