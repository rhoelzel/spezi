#include "BitBoardArray.hpp"
#include "MoveGeneration.hpp"
#include "Position.hpp"
#include "Color.hpp"

#include <array>
#include <iostream>

using namespace spezi;

int counter = 0;
int maxDepth = 1;

auto constexpr OnePlyMaxSize = 220*MoveSize;

std::array<Square, OnePlyMaxSize * 20> moveList {};  // will be able to store 20-ply deep move lists

// pointers to the two bruteForce template specializations,
// need to be initialized at runtime 
void (*f[NumberOfColors])(Position &, int);

template<Color color> 
void bruteForce(Position & p, int depth)
{
    auto const firstMove = moveList.data() + depth * OnePlyMaxSize;
    auto const lastMove = allMoves<color>(p, firstMove);  

    counter += (lastMove-firstMove) / MoveSize;

    if(++depth == maxDepth)
    {
        return;
    }

    auto constexpr other = (color + 1) % NumberOfColors;

    MoveAddress nextMove;
    for(nextMove = firstMove; nextMove[CAPTURED]!=KING; nextMove += MoveSize)
    {
        move<color, CAPTURE>(p, nextMove);
        f[other](p, depth);
        unmove<color, CAPTURE>(p, nextMove);
    }
    for(; nextMove!=lastMove; nextMove += MoveSize)
    {
        move<color, NON_CAPTURE>(p, nextMove);
        f[other](p, depth);
        unmove<color, NON_CAPTURE>(p, nextMove);
    }
}

int main(int argc, char** argv)
{
    if(argc > 1)
    {
        maxDepth = std::stoi(argv[1]);
    }

    f[WHITE] = bruteForce<WHITE>;
    f[BLACK] = bruteForce<BLACK>;

    auto p = StartPosition;
    bruteForce<WHITE>(p, 0);

    prettyPrint(p);
    prettyPrint(moveList.data());
    std::cout<<"Nodes: "<<counter;
}
