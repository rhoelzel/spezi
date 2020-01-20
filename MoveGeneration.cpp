#include "MoveGeneration.hpp"
#include "BitBoardArray.hpp"

namespace spezi
{
    int isLegal(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return 0;
    }
    
    MoveAddress pawnCaptures(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress knightCaptures(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress bishopCaptures(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress rookCaptures(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress queenCaptures(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress kingCaptures(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }
    
    MoveAddress queenMoves(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress rookMoves(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress bishopMoves(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress knightMoves(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress pawnMoves(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }

    MoveAddress kingMoves(Position & position, Color const toMove, MoveAddress nextMove)
    {
        return nextMove;
    }
    
    MoveList allMoves(Position & position, Color const toMove)
    {
        MoveList result;
        return result;
    }
}