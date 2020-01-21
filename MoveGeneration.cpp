#include "MoveGeneration.hpp"
#include "BitBoardArray.hpp"

#include <iostream>

namespace spezi
{
    MoveList allMoves(Position & position, Color const /* toMove */)
    {
        MoveList result;

        auto nextMove = result.data();

        nextMove = generateRegularMovesBy<PAWN, WHITE, true>(position, nextMove);
        nextMove = generateRegularMovesBy<PAWN, WHITE, false>(position, nextMove);  
        nextMove = generateRegularMovesBy<KNIGHT, WHITE>(position, nextMove);
        nextMove = generateSlidingMovesBy<BISHOP, WHITE>(position, nextMove);
        nextMove = generateSlidingMovesBy<ROOK, WHITE>(position, nextMove);
        nextMove = generateSlidingMovesBy<QUEEN, WHITE>(position, nextMove);
        nextMove = generateRegularMovesBy<KING, WHITE>(position, nextMove);
        *nextMove = NULL_SQUARE;

        return result;
    }

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