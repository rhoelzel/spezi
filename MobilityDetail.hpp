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
    auto averageMobility(Square square, Square const population, double factor, 
        std::mt19937_64 & gen, std::uniform_int_distribution<Square> & dis, double const max = 500000.)
    {
        if constexpr(piece == PAWN)
        {
            if(square > h7 || square < a2)
            {
                return 0.0;
            }            
        }

        auto result = 0.;

        for(auto i = 0;i<max;++i)
        {
            auto const bb = random(square, population, gen, dis);
                  
            auto const reached1 = reachable<piece>(square, bb);
            result += popcount(reached1);

            auto promotion = EMPTY;

            auto result2 = 0.;
            auto reached2 = EMPTY;
            auto init2 = reached1 & ~bb;
            if constexpr(piece == PAWN)
            {
                init2 = reached1;
                promotion |= init2 & RANKS[SquaresPerFile-1];
            }
            while(init2)
            {      
                auto const square2 = ffs(init2);
                auto const reached = promotion ? reachable<QUEEN>(square2, bb) : reachable<piece>(square2, bb);
                result2 += popcount(reached);
                reached2 |= reached;
                init2 &= (init2-1);
            }
            if(reached2 & ~reached1 & ~bb)
            {
                result += result2/popcount(reached1 & ~bb)*factor;
            }

            auto result3 = 0.;
            auto reached3 = EMPTY;
            auto init3 = reached2 & ~reached1 & ~bb;
            if constexpr(piece == PAWN)
            {
                promotion |= init3 & RANKS[SquaresPerFile-1];
            }
            while(init3)
            {      
                auto const square3 = ffs(init3);
                auto const reached = promotion ? reachable<QUEEN>(square3, bb) : reachable<piece>(square3,bb);
                result3 += popcount(reached);
                reached3 |= reached;
                init3 &= (init3-1);
            }
            if(reached3 & ~reached2 & ~reached1 & ~bb)
            {
                result += result3/popcount(reached2 & ~reached1 & ~bb)*factor*factor;
            }
            
            auto result4 = 0.;
            auto reached4 = EMPTY;
            auto init4 = reached3 & ~reached2 & ~reached1 & ~bb;
            if constexpr(piece == PAWN)
            {
                promotion |= init4 & RANKS[SquaresPerFile-1];
            }
            while(init4)
            {      
                auto const square4 = ffs(init4);
                auto const reached = promotion ? reachable<QUEEN>(square4, bb) : reachable<piece>(square4,bb);
                result4 += popcount(reached);
                reached4 |= reached;
                init4 &= (init4-1);
            }
            if(reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)
            {
                result += result4/popcount(reached3 & ~reached2 & ~reached1 & ~bb)*factor*factor*factor;
            }

            auto result5 = 0.;
            auto reached5 = EMPTY;
            auto init5 = reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb;
            if constexpr(piece == PAWN)
            {
                promotion |= init5 & RANKS[SquaresPerFile-1];
            }
            while(init5)
            {      
                auto const square5 = ffs(init5);
                auto const reached = promotion ? reachable<QUEEN>(square5, bb) : reachable<piece>(square5,bb);
                result5 += popcount(reached);
                reached5 |= reached;
                init5 &= (init5-1);
            }
            if(reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)
            {
                result += result5/popcount(reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)*factor*factor*factor*factor;
            }

            auto result6 = 0.;
            auto reached6 = EMPTY;
            auto init6 = reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb;
            if constexpr(piece == PAWN)
            {
                promotion |= init6 & RANKS[SquaresPerFile-1];
            }
            while(init6)
            {      
                auto const square6 = ffs(init6);
                auto const reached = promotion ? reachable<QUEEN>(square6, bb) : reachable<piece>(square6,bb);
                result6 += popcount(reached);
                reached6 |= reached;
                init6 &= (init6-1);
            }
            if(reached6 & ~reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)
            {
                result += result6/popcount(reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)*factor*factor*factor*factor*factor;
            }

            auto result7 = 0.;
            auto reached7 = EMPTY;
            auto init7 = reached6 & ~reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb;
            if constexpr(piece == PAWN)
            {
                promotion |= init7 & RANKS[SquaresPerFile-1];
            }
            while(init7)
            {      
                auto const square7 = ffs(init7);
                auto const reached = promotion ? reachable<QUEEN>(square7, bb) : reachable<piece>(square7,bb);
                result7 += popcount(reached);
                reached7 |= reached;
                init7 &= (init7-1);
            }
            if(reached7 & ~reached6 & ~reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)
            {
                result += result7/popcount(reached6 & ~reached5 & ~reached4 & ~reached3 & ~reached2 & ~reached1 & ~bb)*factor*factor*factor*factor*factor*factor;
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

    // logic above here is only used to generate MobilityCoefficients.hpp (via polynomial fitting)
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

    template<Piece piece>
    auto constexpr averageMobilities()
    { 
        std::array<std::array<float, MaxBoardPopulation-2>, NumberOfSquares> result {};
        for(auto square = a1; square != OFF_BOARD; ++square)
        {
            for(auto population = 3; population <= MaxBoardPopulation; ++population)
            {
                auto const resultUnscaled = averageMobility(piece, square, population);
         
                auto const referenceMobility = averageMobility(PAWN, d4, 16);
                result[square][population-3] = resultUnscaled/referenceMobility;
            }
        }
        return result;
    }
}
