#include "Mobility.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>

namespace spezi
{
    namespace detail
    {
        BitBoard random(Square const square, Square const population, 
        std::mt19937_64 & gen, std::uniform_int_distribution<Square> & dis)
        {
            auto result = SQUARES[square];
            while(popcount(result)!= population)
            {
                result |= A1 << dis(gen);
            }
            return result;
        }

        void dumpMobilities(double const factor, double const max)
        {
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<Square> dis(0 , NumberOfSquares-1);
    
            for(auto population = 32; population > 2; --population)
            {
                for(auto square = a1; square != OFF_BOARD; ++square)
                {
                    std::cout<<detail::averageMobility<PAWN>(square, population, factor, gen, dis, max)<<", ";
                    std::cout<<detail::averageMobility<KNIGHT>(square, population, factor, gen, dis, max)<<", ";
                    std::cout<<detail::averageMobility<BISHOP>(square, population, factor, gen, dis, max)<<", ";
                    std::cout<<detail::averageMobility<ROOK>(square, population, factor, gen, dis, max)<<", ";
                    std::cout<<detail::averageMobility<QUEEN>(square, population, factor, gen, dis, max)<<", ";
                    std::cout<<detail::averageMobility<KING>(square, population, factor, gen, dis, max)<<", ";            
                }
                std::cout<<std::endl;
                std::cerr<<"finished "<<population<<std::endl;
            }
        }
    }

    void prettyPrint(MobilityArray const & a, Square const population)
    {          
        auto const p = populationIndex(population);
        std::cout<<"++++++++++"<<std::endl;
        std::cout<<std::fixed<<std::setprecision(2)<<std::setw(4)
        <<milliToPawnUnit(a[a8][p])<<" | "<<milliToPawnUnit(a[b8][p])<<" | "<<milliToPawnUnit(a[c8][p])<<" | "<<milliToPawnUnit(a[d8][p])<<" | "<<milliToPawnUnit(a[e8][p])<<" | "<<milliToPawnUnit(a[f8][p])<<" | "<<milliToPawnUnit(a[g8][p])<<" | "<<milliToPawnUnit(a[h8][p])<<std::endl
        <<milliToPawnUnit(a[a7][p])<<" | "<<milliToPawnUnit(a[b7][p])<<" | "<<milliToPawnUnit(a[c7][p])<<" | "<<milliToPawnUnit(a[d7][p])<<" | "<<milliToPawnUnit(a[e7][p])<<" | "<<milliToPawnUnit(a[f7][p])<<" | "<<milliToPawnUnit(a[g7][p])<<" | "<<milliToPawnUnit(a[h7][p])<<std::endl
        <<milliToPawnUnit(a[a6][p])<<" | "<<milliToPawnUnit(a[b6][p])<<" | "<<milliToPawnUnit(a[c6][p])<<" | "<<milliToPawnUnit(a[d6][p])<<" | "<<milliToPawnUnit(a[e6][p])<<" | "<<milliToPawnUnit(a[f6][p])<<" | "<<milliToPawnUnit(a[g6][p])<<" | "<<milliToPawnUnit(a[h6][p])<<std::endl
        <<milliToPawnUnit(a[a5][p])<<" | "<<milliToPawnUnit(a[b5][p])<<" | "<<milliToPawnUnit(a[c5][p])<<" | "<<milliToPawnUnit(a[d5][p])<<" | "<<milliToPawnUnit(a[e5][p])<<" | "<<milliToPawnUnit(a[f5][p])<<" | "<<milliToPawnUnit(a[g5][p])<<" | "<<milliToPawnUnit(a[h5][p])<<std::endl
        <<milliToPawnUnit(a[a4][p])<<" | "<<milliToPawnUnit(a[b4][p])<<" | "<<milliToPawnUnit(a[c4][p])<<" | "<<milliToPawnUnit(a[d4][p])<<" | "<<milliToPawnUnit(a[e4][p])<<" | "<<milliToPawnUnit(a[f4][p])<<" | "<<milliToPawnUnit(a[g4][p])<<" | "<<milliToPawnUnit(a[h4][p])<<std::endl
        <<milliToPawnUnit(a[a3][p])<<" | "<<milliToPawnUnit(a[b3][p])<<" | "<<milliToPawnUnit(a[c3][p])<<" | "<<milliToPawnUnit(a[d3][p])<<" | "<<milliToPawnUnit(a[e3][p])<<" | "<<milliToPawnUnit(a[f3][p])<<" | "<<milliToPawnUnit(a[g3][p])<<" | "<<milliToPawnUnit(a[h3][p])<<std::endl
        <<milliToPawnUnit(a[a2][p])<<" | "<<milliToPawnUnit(a[b2][p])<<" | "<<milliToPawnUnit(a[c2][p])<<" | "<<milliToPawnUnit(a[d2][p])<<" | "<<milliToPawnUnit(a[e2][p])<<" | "<<milliToPawnUnit(a[f2][p])<<" | "<<milliToPawnUnit(a[g2][p])<<" | "<<milliToPawnUnit(a[h2][p])<<std::endl
        <<milliToPawnUnit(a[a1][p])<<" | "<<milliToPawnUnit(a[b1][p])<<" | "<<milliToPawnUnit(a[c1][p])<<" | "<<milliToPawnUnit(a[d1][p])<<" | "<<milliToPawnUnit(a[e1][p])<<" | "<<milliToPawnUnit(a[f1][p])<<" | "<<milliToPawnUnit(a[g1][p])<<" | "<<milliToPawnUnit(a[h1][p])<<std::endl
        <<std::endl;
    }

    void prettyPrint(MobilityDistribution const & d)
    {
        std::cout<<"..........."<<std::endl;
        for(auto population = 32; population > 22; --population)
        {
            std::cout<<std::fixed<<std::setprecision(2)<<std::setw(4)
                <<population<<":\t"<<milliToPawnUnit(d[population-3])<<" | "
                <<population-10<<":\t"<<milliToPawnUnit(d[population-13])<<" | "
                <<population-20<<":\t"<<milliToPawnUnit(d[population-23])<<std::endl;
        }
    }
}