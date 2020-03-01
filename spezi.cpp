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
    Position p;//(fen);
        
    p.evaluate(depth);
    std::cout<<p.getFen()<<std::endl;;
}
    
