#include "BitBoardArray.hpp"

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
        
        constexpr BitBoard rank(Square const square)
        {
            auto const rank = square / NumberOfRanks;

            auto result = A1 << (rank * NumberOfFiles);     // square on A file of same rank
            result |= (result << 1);                        // square on B file
            result |= (result << 2);                        // squares on C,D files 
            result |= (result << 4);                        // squares on E,F,G,H files

            return result;
        }

        constexpr BitBoard file(Square const square)
        {
            auto const file = square % NumberOfFiles; 

            auto result = A1 << file;                       // square on 1st rank of same file    
            result |= (result << (1*NumberOfFiles));        // square on 2nd rank
            result |= (result << (2*NumberOfFiles));        // squares on 3rd and 4th ranks 
            result |= (result << (4*NumberOfFiles));        // squares on 5th through 8th ranks
            
            return result;
        }

        constexpr BitBoard rankAndFile(Square const square)
        {
            return rank(square) | file(square);            
        }

        constexpr BitBoard diagonals(Square const square)
        {
            auto result = EMPTY;
            for(int i = 1-NumberOfFiles; i < NumberOfFiles; ++i)
            {
                result |= toBitBoard(shift(square, i, i));
                result |= toBitBoard(shift(square, -i, i));
            }

            return result;    
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

        constexpr BitBoard rookMask(Square const square)
        {
            return ((rank(square) & ~FILES[0] & ~FILES[NumberOfFiles-1]) 
                    | (file(square) & ~RANKS[0] & ~RANKS[NumberOfRanks-1]))
                & (~toBitBoard(square));
        }

        constexpr BitBoard bishopMask(Square const square)
        {
            return diagonals(square) & INNER & (~toBitBoard(square));    
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

        auto constexpr AllSquares = std::make_integer_sequence<Square, NumberOfSquares>{};
    }

    BitBoardArray const Ranks = collectBitBoards<rank>(AllSquares);
    BitBoardArray const Files = collectBitBoards<file>(AllSquares);
    BitBoardArray const RanksAndFiles = collectBitBoards<rankAndFile>(AllSquares);
    BitBoardArray const Diagonals = collectBitBoards<diagonals>(AllSquares);

    BitBoardArray const KingMoveAttacks = collectBitBoards<kingMoveAttack>(AllSquares);
    BitBoardArray const RookMasks = collectBitBoards<rookMask>(AllSquares);
    BitBoardArray const BishopMasks = collectBitBoards<bishopMask>(AllSquares);
    BitBoardArray const KnightMoveAttacks = collectBitBoards<knightMoveAttack>(AllSquares);
}