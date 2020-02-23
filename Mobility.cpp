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
    }

    long toCentiPawns(Mobility const mobility)
    {
        return std::lround(static_cast<double>(mobility) * 100.);
    }

    float toPawns(Mobility const mobility)
    {
        return mobility;
    }

    void prettyPrint(MobilityArray const & a, Square const p)
    {      
        
        std::cout<<"++++++++++"<<std::endl;
        std::cout<<std::fixed<<std::setprecision(2)<<std::setw(4)
        <<toPawns(a[a8][p-3])<<" | "<<toPawns(a[b8][p-3])<<" | "<<toPawns(a[c8][p-3])<<" | "<<toPawns(a[d8][p-3])<<" | "<<toPawns(a[e8][p-3])<<" | "<<toPawns(a[f8][p-3])<<" | "<<toPawns(a[g8][p-3])<<" | "<<toPawns(a[h8][p-3])<<std::endl
        <<toPawns(a[a7][p-3])<<" | "<<toPawns(a[b7][p-3])<<" | "<<toPawns(a[c7][p-3])<<" | "<<toPawns(a[d7][p-3])<<" | "<<toPawns(a[e7][p-3])<<" | "<<toPawns(a[f7][p-3])<<" | "<<toPawns(a[g7][p-3])<<" | "<<toPawns(a[h7][p-3])<<std::endl
        <<toPawns(a[a6][p-3])<<" | "<<toPawns(a[b6][p-3])<<" | "<<toPawns(a[c6][p-3])<<" | "<<toPawns(a[d6][p-3])<<" | "<<toPawns(a[e6][p-3])<<" | "<<toPawns(a[f6][p-3])<<" | "<<toPawns(a[g6][p-3])<<" | "<<toPawns(a[h6][p-3])<<std::endl
        <<toPawns(a[a5][p-3])<<" | "<<toPawns(a[b5][p-3])<<" | "<<toPawns(a[c5][p-3])<<" | "<<toPawns(a[d5][p-3])<<" | "<<toPawns(a[e5][p-3])<<" | "<<toPawns(a[f5][p-3])<<" | "<<toPawns(a[g5][p-3])<<" | "<<toPawns(a[h5][p-3])<<std::endl
        <<toPawns(a[a4][p-3])<<" | "<<toPawns(a[b4][p-3])<<" | "<<toPawns(a[c4][p-3])<<" | "<<toPawns(a[d4][p-3])<<" | "<<toPawns(a[e4][p-3])<<" | "<<toPawns(a[f4][p-3])<<" | "<<toPawns(a[g4][p-3])<<" | "<<toPawns(a[h4][p-3])<<std::endl
        <<toPawns(a[a3][p-3])<<" | "<<toPawns(a[b3][p-3])<<" | "<<toPawns(a[c3][p-3])<<" | "<<toPawns(a[d3][p-3])<<" | "<<toPawns(a[e3][p-3])<<" | "<<toPawns(a[f3][p-3])<<" | "<<toPawns(a[g3][p-3])<<" | "<<toPawns(a[h3][p-3])<<std::endl
        <<toPawns(a[a2][p-3])<<" | "<<toPawns(a[b2][p-3])<<" | "<<toPawns(a[c2][p-3])<<" | "<<toPawns(a[d2][p-3])<<" | "<<toPawns(a[e2][p-3])<<" | "<<toPawns(a[f2][p-3])<<" | "<<toPawns(a[g2][p-3])<<" | "<<toPawns(a[h2][p-3])<<std::endl
        <<toPawns(a[a1][p-3])<<" | "<<toPawns(a[b1][p-3])<<" | "<<toPawns(a[c1][p-3])<<" | "<<toPawns(a[d1][p-3])<<" | "<<toPawns(a[e1][p-3])<<" | "<<toPawns(a[f1][p-3])<<" | "<<toPawns(a[g1][p-3])<<" | "<<toPawns(a[h1][p-3])<<std::endl
        <<std::endl;
    }

    void prettyPrint(MobilityDistribution const & d)
    {
        std::cout<<"..........."<<std::endl;
        for(auto population = 32; population > 22; --population)
        {
            std::cout<<std::fixed<<std::setprecision(2)<<std::setw(4)
                <<population<<":\t"<<toPawns(d[population-3])<<" | "
                <<population-10<<":\t"<<toPawns(d[population-13])<<" | "
                <<population-20<<":\t"<<toPawns(d[population-23])<<std::endl;
        }
    }
}