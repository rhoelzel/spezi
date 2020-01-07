#include "Magic.hpp"

#include "Common.hpp"

#include <utility>

namespace spezi
{
    namespace
    {
        constexpr int numberOfSetBits(BitBoard bitBoard)
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

        auto constexpr AllSquares = std::make_integer_sequence<Square, NumberOfSquares>{};


        auto constexpr j = numberOfSetBits(0xFFFF00FF);
    }
} 
    
