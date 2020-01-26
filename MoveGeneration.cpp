#include "MoveGeneration.hpp"
#include "BitBoardArray.hpp"

#include <iostream>
#include <string>

namespace spezi
{
    void prettyPrint(MoveAddress const firstMove)
    {
        for(auto nextMove = firstMove; *nextMove != NULL_SQUARE; nextMove += MoveSize)
        {
            std::cout<<PieceTags[nextMove[PIECE]]
                <<static_cast<char>(nextMove[FROM] % SquaresPerRank + 0x61)
                <<static_cast<char>(nextMove[FROM] / SquaresPerFile + 0x31)
                <<(nextMove[CAPTURED] != KING ? std::string("x") + PieceTags[nextMove[CAPTURED]] : "")
                <<static_cast<char>(nextMove[TO] % SquaresPerRank + 0x61)
                <<static_cast<char>(nextMove[TO] / SquaresPerFile + 0x31)
                <<std::endl;
        }       
    }
}