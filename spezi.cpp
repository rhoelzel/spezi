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
    if(argc > 1)
    {
        maxDepth = std::stoi(argv[1]);
    }

    auto depth = maxDepth;
    auto initial = 20;
    auto step = 10;

    if(argc > 2)
    {
        initial  = std::stoi(argv[2]);
    }

    if(argc > 3)
    {
        step = std::stoi(argv[3]);
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

    for(int i = 1; i!=31;++i)
    {
        auto const k = detail::averageMobilities<KING>(32-i);
        auto const n = detail::averageMobilities<KNIGHT>(32-i);
        auto const b = detail::averageMobilities<BISHOP>(32-i);
        auto const r = detail::averageMobilities<ROOK>(32-i);
        auto const q = detail::averageMobilities<QUEEN>(32-i);
        auto const p = detail::averageMobilities<PAWN>(32-i);

        for(auto j = 0; j< NumberOfSquares; ++j)
        {
            std::cout<<k[j]<<", "<<n[j]<<", "<<b[j]<<", "<<r[j]<<", "<<q[j]<<", "<<p[j]<<", ";
        }
        std::cout<<std::endl;
        std::cerr<<"finished "<<i<<std::endl;
    }
}
