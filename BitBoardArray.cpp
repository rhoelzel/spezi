#include "BitBoardArray.hpp"

namespace spezi
{
    namespace
    {
        Square constexpr shift(Square const square, int8_t const rankShift, int8_t const fileShift)
        {
            if(square % SquaresPerRank + fileShift >= SquaresPerRank
                || square % SquaresPerRank + fileShift < 0
                || square / SquaresPerFile + rankShift >= SquaresPerFile
                || square / SquaresPerFile + rankShift < 0)
            {
                return -1;
            }

            return square + SquaresPerRank*rankShift + fileShift;            
        }

        BitBoard constexpr toBitBoard(Square const square)
        {
            if(square < 0 || square >= NumberOfSquares)
            {
                return EMPTY;
            }

            return A1 << square;
        }
        
        BitBoard constexpr rank(Square const square)
        {
            auto const rank = square / SquaresPerFile;

            auto result = A1 << (rank * SquaresPerRank);    // square on A file of same rank
            result |= (result << 1);                        // square on B file
            result |= (result << 2);                        // squares on C,D files 
            result |= (result << 4);                        // squares on E,F,G,H files

            return result;
        }

        BitBoard constexpr file(Square const square)
        {
            auto const file = square % SquaresPerRank; 

            auto result = A1 << file;                       // square on 1st rank of same file    
            result |= (result << (1*SquaresPerRank));       // square on 2nd rank
            result |= (result << (2*SquaresPerRank));       // squares on 3rd and 4th ranks 
            result |= (result << (4*SquaresPerRank));       // squares on 5th through 8th ranks
            
            return result;
        }

        BitBoard constexpr rankAndFile(Square const square)
        {
            return rank(square) | file(square);            
        }

        BitBoard constexpr diagonals(Square const square)
        {
            auto result = EMPTY;
            for(int i = 1-SquaresPerFile; i < SquaresPerFile; ++i)
            {
                result |= toBitBoard(shift(square, i, i));
                result |= toBitBoard(shift(square, -i, i));
            }

            return result;    
        }

        BitBoard constexpr kingMoveAttack(Square const square)
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

        BitBoard constexpr rookMask(Square const square)
        {
            return ((rank(square) & ~FILES[0] & ~FILES[SquaresPerRank-1]) 
                    | (file(square) & ~RANKS[0] & ~RANKS[SquaresPerFile-1]))
                & (~toBitBoard(square));
        }

        BitBoard constexpr bishopMask(Square const square)
        {
            return diagonals(square) & INNER & (~toBitBoard(square));    
        }

        BitBoard constexpr knightMoveAttack(Square const square)
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

        BitBoard constexpr whitePawnMove(Square const square)
        {
            auto result = toBitBoard(shift(square, 1, 0));
            if(square / SquaresPerRank == 1)
            {
                result |= toBitBoard(shift(square, 2, 0));
            }
        
            return result & ~RANKS[1];
        }

        BitBoard constexpr whitePawnAttack(Square const square)
        {
            auto result = toBitBoard(shift(square, 1, -1));
            result |= toBitBoard(shift(square, 1, 1));
        
            return result & ~RANKS[1];
        }

        BitBoard constexpr blackPawnMove(Square const square)
        {
            auto result = toBitBoard(shift(square, -1, 0));
            if(square / SquaresPerRank == SquaresPerFile-2)
            {
                result |= toBitBoard(shift(square, -2, 0));
            }
        
            return result & ~RANKS[SquaresPerFile-2];
        }

        BitBoard constexpr blackPawnAttack(Square const square)
        {
            auto result = toBitBoard(shift(square, -1, -1));
            result |= toBitBoard(shift(square, -1, 1));
       
            return result & ~RANKS[SquaresPerFile-2];
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
    BitBoardArray const WhitePawnMoves = collectBitBoards<whitePawnMove>(AllSquares);
    BitBoardArray const WhitePawnAttacks = collectBitBoards<whitePawnAttack>(AllSquares);
    BitBoardArray const BlackPawnMoves = collectBitBoards<blackPawnMove>(AllSquares); 
    BitBoardArray const BlackPawnAttacks = collectBitBoards<blackPawnAttack>(AllSquares);
}