#include "MoveGeneration.hpp"
#include "BitBoardArray.hpp"

#include <iostream>
#include <string>

#define ffs(X) X

namespace spezi
{
    void prettyPrint(MoveAddress const firstMove)
    {
        for(auto nextMove = firstMove; *nextMove != NULL_SQUARE; nextMove += MoveSize)
        {
            std::cout<<PieceTags[nextMove[PIECE]]
                <<static_cast<char>(ffs(nextMove[FROM]) % SquaresPerRank + 0x61)
                <<static_cast<char>(ffs(nextMove[FROM]) / SquaresPerFile + 0x31)
                <<(nextMove[CAPTURED] != NULL_PIECE? std::string("x") : "")
                <<static_cast<char>(ffs(nextMove[TO]) % SquaresPerRank + 0x61)
                <<static_cast<char>(ffs(nextMove[TO]) / SquaresPerFile + 0x31)
                <<(nextMove[PROMOTED] != NULL_PIECE ? PieceTags[nextMove[PROMOTED]] : "")
                <<std::endl;
        }       
    }
}