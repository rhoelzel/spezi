#include "Position.hpp"

#include <array>
#include <iostream>

namespace spezi
{
    namespace
    {
        auto constexpr NO_PIECE = NumberOfPieceTypes * 2;
        auto constexpr PIECE_ERROR = NumberOfPieceTypes * 2 + 1;
        
        auto constexpr pieceBoard(Position const & position)
        {
            std::array<int, NumberOfSquares> retval {};
            
            for(auto const square : SQUARES)
            {
                auto & result = retval[ffs(square)];
                result = NO_PIECE;    
                int pieces = 0;

                for(auto const piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING })
                {
                    if(position.individualPieces[piece] & square)
                    {
                        if(position.allPieces[WHITE] & square)
                        {
                            result = piece;
                        }
                        else
                        {
                            result = NumberOfPieceTypes + piece;
                        }
                        
                        ++ pieces;
                    }
                }

                switch(pieces)
                {
                case 0:
                    if((position.allPieces[WHITE]
                        | position.allPieces[BLACK]
                        | ~position.empty) & square)
                        {
                            result = PIECE_ERROR;
                        }
                    break;
                case 1:
                    if(!((position.allPieces[WHITE]
                        ^ position.allPieces[BLACK])
                        & ~position.empty & square))
                        {
                            result = PIECE_ERROR;
                        }
                    break;
                default:
                    result = PIECE_ERROR;
                    break;
                }
            }
            return retval;
        }
    } 

    void prettyPrint(Position const & position)
    {
        auto const board = pieceBoard(position);
        
        unsigned char const p[] = {'*', 'N', 'B', 'R', 'Q', 'K','+', 'n', 'b', 'r', 'q', 'k', '.', 'E'};         
        unsigned char const v = '|'; unsigned char const h = '-';
        unsigned char const ul = '/'; unsigned char const ur = '\\';
        unsigned char const ll = '\\'; unsigned char const lr = '/';
        
        auto const bar =  std::string(17, h);
        
        std::cout<<"  a b c d e f g h"<<std::endl;
        std::cout<<ul<<bar<<ur;
        for(int rank = 7; rank >= 0; --rank)
        {
            std::cout<<std::endl<<v;
            for(int file = 0; file < 8; ++file)
            {
                std::cout<<" "<<p[board[rank*8+file]];
            } 
            std::cout<<' '<<v<<' '<<std::to_string(rank+1);
        }
        std::cout<<std::endl<<ll<<bar<<lr<<std::endl;
        std::cout<<"  a b c d e f g h"<<std::endl;        
    }
}