#include "Attacks.hpp"
#include "Constants.hpp"

namespace spezi
{
    namespace
    {
        using Square = int;

        constexpr Square shift(Square const square, int8_t rankShift, int8_t fileShift)
        {
            if(square >= NumberOfSquares
                || square < 0
                || square + rankShift*NumberOfFiles + fileShift >= NumberOfSquares
                || square + rankShift*NumberOfFiles + fileShift < 0)
            {
                return -1;
            }

            return square + NumberOfFiles*rankShift + fileShift;            
        }

        constexpr BitBoard toBitBoard(Square const square)
        {
            if(square < 0)
            {
                return EMPTY;
            }
            return 1 << square;
        }
        
        constexpr BitBoard kingAttacks(Square const square)
        {
            auto result = toBitBoard(shift(square, -1, -1));  
            result |= toBitBoard(shift(square, -1, 0));  
            result |= toBitBoard(shift(square, -1, 1));  
            result |= toBitBoard(shift(square, 0, -1));  
            result |= toBitBoard(shift(square, 0, 1));  
            result |= toBitBoard(shift(square, 1, -1));  
            result |= toBitBoard(shift(square, 1, 0));  
            result |= toBitBoard(shift(square, 1, 1));  
            return result;
        }

        constexpr BitBoard knightAttacks(Square const square)
        {
            auto result = toBitBoard(shift(square, -1, -2));  
            result |= toBitBoard(shift(square, -2, -1));  
            result |= toBitBoard(shift(square, -2, 1));  
            result |= toBitBoard(shift(square, -1, 2));  
            result |= toBitBoard(shift(square, 1, 2));  
            result |= toBitBoard(shift(square, 2, 1));  
            result |= toBitBoard(shift(square, 2, -1));  
            result |= toBitBoard(shift(square, 1, -2));  
            return result;
        }

        constexpr BitBoard whitePawnMoves(Square const square)
        {
            auto result = toBitBoard(shift(square, 1, 0));
        }
        
        constexpr BitBoard whitePawnAttacks(Square const square)
        {
            auto result = toBitBoard(shift(square, 1, -1));
            result |= toBitBoard(shift(square, 1, 1));
        }

        constexpr BitBoard blackPawnMoves(Square const square)
        {
            auto result = toBitBoard(shift(square,-1, 0));
        }

        constexpr BitBoard blackPawnAttacks(Square const square)
        {
            auto result = toBitBoard(shift(square, -1, -1));
            result |= toBitBoard(shift(square, -1, 1));
        }

    }
}