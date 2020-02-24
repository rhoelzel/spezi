#include "BitBoardArray.hpp"
#include "Color.hpp"
#include "Mobility.hpp"
#include "MoveGeneration.hpp"
#include "Position.hpp"


#include <array>
#include <iostream>

using namespace spezi;

int counter = 0;
int maxDepth = 3;

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
        
    auto constexpr other = (color == WHITE) ? BLACK : WHITE;

    MoveAddress nextMove;

    for(nextMove = firstMove; nextMove[CAPTURED]!=NULL_PIECE; nextMove += MoveSize)
    {
        move<color, CAPTURE>(p, nextMove);
        f[other](p, depth);
        move<color, CAPTURE>(p, nextMove);
    }
    for(; nextMove!=lastMove; nextMove += MoveSize)
    {
        move<color, NON_CAPTURE>(p, nextMove);
        f[other](p, depth);
        move<color, NON_CAPTURE>(p, nextMove);
    }     
}

int main(int argc, char** argv)
{
    double factor = 0.5;
    double max = 20000;
    if(argc > 1)
    {
        factor = std::stod(argv[1]);
    }
    if(argc > 2)
    {
        max = std::stod(argv[2]);
    }
    
    /*
    f[WHITE] = bruteForce<WHITE>;
    f[BLACK] = bruteForce<BLACK>;

    auto p = StartPosition;
    
    Square myList[4*MoveSize] = 
    { a2, b7, PAWN, PAWN, NULL_PIECE,
      b8, a1, KNIGHT, ROOK, NULL_PIECE,
      e2, e7, PAWN, PAWN, NULL_PIECE,
      a7, a2, PAWN, PAWN, NULL_PIECE };
    
    move<WHITE, CAPTURE>(p, &myList[0]);
    move<BLACK, CAPTURE>(p, &myList[5]);
    move<WHITE, CAPTURE>(p, &myList[10]);
    move<BLACK, NON_CAPTURE>(p, &myList[15]);
    
    bruteForce<WHITE>(p, 0);
    
    prettyPrint(p);
    allMoves<WHITE>(p, moveList.data());

    prettyPrint(moveList.data());
    std::cout<<"Nodes: "<<counter;*/
   
    
}
    
