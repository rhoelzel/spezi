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

void dumpMobilities(double const factor, double const max)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<Square> dis(0 , NumberOfSquares-1);
    
    for(auto population = 32; population > 2; --population)
    {
        for(auto square = a1; square != OFF_BOARD; ++square)
        {
            std::cout<<detail::averageMobility<PAWN>(square, population, factor, gen, dis, max)<<", ";
            std::cout<<detail::averageMobility<KNIGHT>(square, population, factor, gen, dis, max)<<", ";
            std::cout<<detail::averageMobility<BISHOP>(square, population, factor, gen, dis, max)<<", ";
            std::cout<<detail::averageMobility<ROOK>(square, population, factor, gen, dis, max)<<", ";
            std::cout<<detail::averageMobility<QUEEN>(square, population, factor, gen, dis, max)<<", ";
            std::cout<<detail::averageMobility<KING>(square, population, factor, gen, dis, max)<<", ";            
        }
        std::cout<<std::endl;
        std::cerr<<"finished "<<population<<std::endl;
    }
}

template<Piece piece, Square pieceSquare, Piece unit, Square unitSquare>
float constexpr convert(Square const population)
{
    return static_cast<float>(AverageMobilities<piece>[pieceSquare][population-3])
        /static_cast<float>(AverageMobilities<unit>[unitSquare][population-3]);
} 

void dump()
{
    for(int p = 32; p>2; --p)
    {
        std::cout<<convert<QUEEN,a1,PAWN,d4>(p)<<", ";
        std::cout<<convert<ROOK,a1,PAWN,d4>(p)<<", ";
        std::cout<<convert<BISHOP,a1,PAWN,d4>(p)<<", ";
        std::cout<<convert<KNIGHT,a1,PAWN,d4>(p)<<", ";
        std::cout<<convert<KING,a1,PAWN,d4>(p)<<", ";
        std::cout<<convert<QUEEN,d4,PAWN,d4>(p)<<", ";
        std::cout<<convert<ROOK,d4,PAWN,d4>(p)<<", ";
        std::cout<<convert<BISHOP,d4,PAWN,d4>(p)<<", ";
        std::cout<<convert<KNIGHT,d4,PAWN,d4>(p)<<", ";
        std::cout<<convert<KING,d4,PAWN,d4>(p)<<std::endl;
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
   
    dump();
   
    /*std::cout<<"pawn c4 ";
    prettyPrint(AverageMobilities<PAWN>[c4]);
    std::cout<<"pawn 32, 16, 3";
    prettyPrint(AverageMobilities<PAWN>, 32);
    prettyPrint(AverageMobilities<PAWN>, 16);
    prettyPrint(AverageMobilities<PAWN>, 3);
    std::cout<<"knight c4 ";
    prettyPrint(AverageMobilities<KNIGHT>[c4]);
    std::cout<<"knight 32,16,3 ";
    prettyPrint(AverageMobilities<KNIGHT>, 32);
    prettyPrint(AverageMobilities<KNIGHT>, 16);
    prettyPrint(AverageMobilities<KNIGHT>, 3);
    std::cout<<"bishop c4 ";
    prettyPrint(AverageMobilities<BISHOP>[c4]);
    std::cout<<"bishop 32,16,3 ";
    prettyPrint(AverageMobilities<BISHOP>, 32);
    prettyPrint(AverageMobilities<BISHOP>, 16);
    prettyPrint(AverageMobilities<BISHOP>, 3);
    std::cout<<"rook c4 ";
    prettyPrint(AverageMobilities<ROOK>[c4]);
    std::cout<<"rook 32,16,3 ";
    prettyPrint(AverageMobilities<ROOK>, 32);
    prettyPrint(AverageMobilities<ROOK>, 16);
    prettyPrint(AverageMobilities<ROOK>, 3);
    std::cout<<"queen c4 ";
    prettyPrint(AverageMobilities<QUEEN>[c4]);
    std::cout<<"queen 32,16,3 ";
    prettyPrint(AverageMobilities<QUEEN>, 32);
    prettyPrint(AverageMobilities<QUEEN>, 16);
    prettyPrint(AverageMobilities<QUEEN>, 3);
    std::cout<<"king c4 ";
    prettyPrint(AverageMobilities<KING>[c4]);
    std::cout<<"king 32,16,3 ";
    prettyPrint(AverageMobilities<KING>, 32);
    prettyPrint(AverageMobilities<KING>, 16);
    prettyPrint(AverageMobilities<KING>, 3);*/

    //dumpMobilities(factor, max);
}
    
