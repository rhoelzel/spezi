#pragma once

#include "BitBoard.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "ZKey.hpp"

namespace spezi
{
    class HashEntry
    {
    public:
        HashEntry(int draft,
                    char castlingBefore,
                    BitBoard enPassantBefore,
                    Square source, 
                    Square target,
                    Piece moved, 
                    Piece captured = KING, // KING: no piece captured 
                    Piece promoted = KING, // KING: no piece promoted
                    char castlingUpdate = 0,
                    BitBoard enPassantAfter = EMPTY
                    );

        ZKey const zKey;
        BitBoard source() const;
        BitBoard target() const;
        Piece moved() const;
        Piece captured() const;
        Piece promoted() const;
        char castlingBefore() const;
        char castlingUpdate() const;
        BitBoard enPassantBefore() const;
        BitBoard enPassantAfter() const;

    private:
        uint_fast64_t move;

        // bits 0-3:    castling flags before move
        static auto constexpr CASTLING_BEFORE_MASK      = 0x000000000000000F;
        // bits 4-9:    en passant square before move 
        static auto constexpr EN_PASSANT_BEFORE_MASK    = 0x00000000000003F0;
        // bits 10-15:  source square
        static auto constexpr SOURCE_SQUARE_MASK        = 0x000000000000FC00;
        // bits 16-21:  target square
        static auto constexpr TARGET_SQUARE_MASK        = 0x00000000003F0000;
        // bits 22-24:  moved piece                
        static auto constexpr MOVED_PIECE_MASK          = 0x0000000001C00000;
        // bits 25-27:  captured piece
        static auto constexpr CAPTURED_PIECE_MASK       = 0x000000000E000000;
        // bits 28-30:  promoted piece  
        static auto constexpr PROMOTED_PIECE_MASK       = 0x0000000070000000;
        // bits 31-34:  castling update flags
        static auto constexpr CASTLING_UPDATE_MASK      = 0x0000000780000000;
        // bits 35-40:  enpassant square after move     
        static auto constexpr EN_PASSANT_AFTER_MASK     = 0x000001F800000000;
        // bits 41-47:  draft                           
        static auto constexpr DRAFT_MASK                = 0x0000FE0000000000;
        // bits 48-63:  score 
        static auto constexpr SCORE_MASK                = 0xFFFF000000000000; 

        static Square constexpr source(uint_fast64_t const move) 
            {return move & SOURCE_SQUARE_MASK;}

        // bits 12-17:  square of captured ep-pawn (if applicable)
        // bits 18-23:  ep-square after move (if applicable)
        // bits

    };
}