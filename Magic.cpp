#include "Magic.hpp"

#include "Common.hpp"

#include <utility>

namespace spezi
{
    namespace
    {
        int constexpr numberOfSetBits(BitBoard bitBoard)
        {
            for(int i = 0;i<NumberOfSquares;++i)
            {
                if(!bitBoard)
                {
                    return i;
                }
                bitBoard &= (bitBoard-1);             
            }

            return NumberOfSquares; 
        }
        
        auto constexpr UpperRight4x4 = std::integer_sequence
        < Square,
            e5, f5, g5, h5, 
            e6, f6, g6, h6,
            e7, f6, g7, h7,
            e8, f8, g8, h8          
        > {};

                

    }
} 
    
