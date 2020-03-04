#include "Position.hpp"

#include <iostream>

using namespace spezi;

int main(int argc, char** argv)
{
    auto depth = 1;

    if(argc > 1)
    {
        depth = std::stoi(argv[1]);
    }
    
    //auto const fen = "r1bqkbnr/1PppPppp/8/8/8/8/pPPP1PPP/nNBQKBNR w KQkq - 0 1";
    //auto const fen = "rnbqkbnr/ppppp11p/5p2/6p1/2N1P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1";
    auto const fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Position p(fen);
        
    p.evaluate(depth);
    std::cout<<p.getFen()<<std::endl;;
}
    
