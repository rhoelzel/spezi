#include "BitBoardArray.hpp"
#include "MoveGeneration.hpp"
#include "Position.hpp"
#include "Color.hpp"

using namespace spezi;

int main()
{
    auto position = StartPosition;
    position.allPieces[WHITE]^=(H2|G3);
    position.individualPieces[PAWN]^=(H2|G3);
    position.empty^=(H2|G3);

    auto const moveList = allMoves<WHITE>(position);
    prettyPrint(moveList);
}
