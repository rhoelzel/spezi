#include "Position.hpp"

#include <iostream>

using namespace spezi;

int main(int argc, char** argv)
{
    auto depth = 0;

    if(argc > 1)
    {
        depth = std::stoi(argv[1]);
    }
    
    auto fen = std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    if(argc == 8)
    {
        fen = "";
        for(int i = 2; i < 8; ++i)
        {
            fen += argv[i];
            fen += ' ';
        }
    }

    Position p(fen);
    std::cout<<p.getBoardDisplay();
    std::cout<<p.getZKey()<<std::endl;
    EvaluationStatistics stats{p.evaluateRecursively(depth)};

    std::cout<<"evaluation:                 "<<stats.evaluation<<std::endl;
    std::cout<<"maximum regular depth:      "<<stats.maximumRegularDepth<<std::endl;
    std::cout<<"maximum reached depth:      "<<stats.maximumReachedDepth<<std::endl;
    std::cout<<"nodes:                      "<<stats.numberOfNodes<<std::endl;
    std::cout<<"quiescence nodes:           "<<stats.numberOfQuiescenceNodes<<std::endl;
    std::cout<<"seconds:                    "<<stats.seconds<<std::endl;
    std::cout<<"nodes per second:           "<<stats.numberOfNodes/stats.seconds<<std::endl;
    std::cout<<p.getZKey()<<std::endl;    
    std::cout<<p.getPrincipalVariation()<<std::endl;
}
    
