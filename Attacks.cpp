#include "Attacks.hpp"

#include <utility>

namespace spezi
{
    namespace
    {
        constexpr Square shift(Square const square, int8_t rankShift, int8_t fileShift)
        {
            if(square % NumberOfFiles + fileShift >= NumberOfFiles
                || square % NumberOfFiles + fileShift < 0
                || square / NumberOfRanks + rankShift >= NumberOfRanks
                || square / NumberOfRanks + rankShift < 0)
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
            return A1 << square;
        }
        
        constexpr BitBoard kingMoveAttack(Square const square)
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

        constexpr BitBoard knightMoveAttack(Square const square)
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

        constexpr BitBoard whitePawnMove(Square const square)
        {
            auto result = toBitBoard(shift(square, 1, 0));
            if(square < NumberOfFiles*2)
            {
                result |= toBitBoard(shift(square, 2, 0));
            }
            return result;
        }

        constexpr BitBoard whitePawnAttack(Square const square)
        {
            auto result = toBitBoard(shift(square, 1, -1));
            result |= toBitBoard(shift(square, 1, 1));
            return result;
        }

        constexpr BitBoard blackPawnMove(Square const square)
        {
            auto result = toBitBoard(shift(square,-1, 0));
            if(square >= NumberOfFiles*(NumberOfRanks-2))
            {
                result |= toBitBoard(shift(square, 2, 0));
            }
            return result;
        }

        constexpr BitBoard blackPawnAttack(Square const square)
        {
            auto result = toBitBoard(shift(square, -1, -1));
            result |= toBitBoard(shift(square, -1, 1));
            return result;
        }

        auto const AllSquares = std::make_integer_sequence<Square, NumberOfSquares>{};

        template <BitBoard bitBoardGenerator(Square const), Square... squares>
        constexpr std::array<BitBoard, sizeof...(squares)> collect(std::integer_sequence<Square, squares...>)
        {
            return std::array<BitBoard, sizeof...(squares)> {bitBoardGenerator(squares)...};            
        }
    }

    std::array<BitBoard, NumberOfSquares> const KingMoveAttacks = collect<kingMoveAttack>(AllSquares);
    std::array<BitBoard, NumberOfSquares> const KnightMoveAttacks = collect<knightMoveAttack>(AllSquares);
}