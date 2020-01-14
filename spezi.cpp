#include "BitBoardArray.hpp"

#include <iostream>

using namespace spezi;

void ppBitBoard(BitBoard b)
{
    std::cout<<"--------\n";
    std::cout<<(b&A8?'X':'*')<<(b&B8?'X':'*')<<(b&C8?'X':'*')<<(b&D8?'X':'*')<<(b&E8?'X':'*')<<(b&F8?'X':'*')<<(b&G8?'X':'*')<<(b&H8?'X':'*')<<'\n';
    std::cout<<(b&A7?'X':'*')<<(b&B7?'X':'*')<<(b&C7?'X':'*')<<(b&D7?'X':'*')<<(b&E7?'X':'*')<<(b&F7?'X':'*')<<(b&G7?'X':'*')<<(b&H7?'X':'*')<<'\n';
    std::cout<<(b&A6?'X':'*')<<(b&B6?'X':'*')<<(b&C6?'X':'*')<<(b&D6?'X':'*')<<(b&E6?'X':'*')<<(b&F6?'X':'*')<<(b&G6?'X':'*')<<(b&H6?'X':'*')<<'\n';
    std::cout<<(b&A5?'X':'*')<<(b&B5?'X':'*')<<(b&C5?'X':'*')<<(b&D5?'X':'*')<<(b&E5?'X':'*')<<(b&F5?'X':'*')<<(b&G5?'X':'*')<<(b&H5?'X':'*')<<'\n';
    std::cout<<(b&A4?'X':'*')<<(b&B4?'X':'*')<<(b&C4?'X':'*')<<(b&D4?'X':'*')<<(b&E4?'X':'*')<<(b&F4?'X':'*')<<(b&G4?'X':'*')<<(b&H4?'X':'*')<<'\n';
    std::cout<<(b&A3?'X':'*')<<(b&B3?'X':'*')<<(b&C3?'X':'*')<<(b&D3?'X':'*')<<(b&E3?'X':'*')<<(b&F3?'X':'*')<<(b&G3?'X':'*')<<(b&H3?'X':'*')<<'\n';
    std::cout<<(b&A2?'X':'*')<<(b&B2?'X':'*')<<(b&C2?'X':'*')<<(b&D2?'X':'*')<<(b&E2?'X':'*')<<(b&F2?'X':'*')<<(b&G2?'X':'*')<<(b&H2?'X':'*')<<'\n';
    std::cout<<(b&A1?'X':'*')<<(b&B1?'X':'*')<<(b&C1?'X':'*')<<(b&D1?'X':'*')<<(b&E1?'X':'*')<<(b&F1?'X':'*')<<(b&G1?'X':'*')<<(b&H1?'X':'*')<<'\n';
}


int main()
{
  for(size_t i = 0; i < KnightMoveAttacks.size();++i)
    {
      ppBitBoard(detail::rookMask(i));
	  }


    
    ppBitBoard(detail::bishopOccupancy(a1,3));   
    ppBitBoard(detail::bishopMoveAttack(a1,3)); std::cout<<std::endl;  
    ppBitBoard(detail::bishopOccupancy(a1,2));   
    ppBitBoard(detail::bishopMoveAttack(a1,2));   std::cout<<std::endl;
    ppBitBoard(detail::bishopOccupancy(a1,4));   
    ppBitBoard(detail::bishopMoveAttack(a1,4));   std::cout<<std::endl;
    ppBitBoard(detail::bishopOccupancy(a1,7));   
    ppBitBoard(detail::bishopMoveAttack(a1,7));   std::cout<<std::endl;
    ppBitBoard(detail::bishopOccupancy(a1,8));   
    ppBitBoard(detail::bishopMoveAttack(a1,8));   std::cout<<std::endl;
    ppBitBoard(detail::bishopOccupancy(d4,0));   
    ppBitBoard(detail::bishopMoveAttack(d4,0));   std::cout<<std::endl;
    ppBitBoard(detail::bishopOccupancy(a1,0b110101000000));
    ppBitBoard(detail::bishopMoveAttack(a1,0b110101000000));      std::cout<<std::endl;
    ppBitBoard(detail::bishopOccupancy(a1,0b110000000000));
    ppBitBoard(detail::bishopMoveAttack(a1,0b110000000000));     std::cout<<std::endl;

}
