#include "MoveGeneration.hpp"
#include "BitBoardArray.hpp"

#include <iostream>

namespace spezi
{
    void prettyPrint(MoveList const & moveList)
    {
        for(auto moveAddress = const_cast<Square *>(moveList.data()); *moveAddress != NULL_SQUARE; moveAddress +=3)
        {
            std::cout<<PieceTags[moveAddress[PIECE]]
                <<static_cast<char>(moveAddress[FROM] % SquaresPerRank + 0x61)
                <<static_cast<char>(moveAddress[FROM] / SquaresPerFile + 0x31)
                <<static_cast<char>(moveAddress[TO] % SquaresPerRank + 0x61)
                <<static_cast<char>(moveAddress[TO] / SquaresPerFile + 0x31)
                <<std::endl;
        }       
    }
}