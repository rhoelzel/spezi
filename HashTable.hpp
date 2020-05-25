#pragma once

#include "BitBoard.hpp"
#include "Mobility.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include "ZKey.hpp"

#include <string>
#include <vector>

namespace spezi
{
    enum HashEntryType
    {
        ALL_NODE = 0,
        PV_NODE = 1,
        CUT_NODE = 2   
    };

    class HashEntry
    {
    public:
        // bits 0-3:    castling flags before move
        static auto constexpr CASTLING_BEFORE_MASK      = 0x000000000000000F;
        // bits 4-9:    en passant square before move 
        static auto constexpr EN_PASSANT_BEFORE_MASK    = 0x00000000000003F0;
        // bits 10-15:  origin square
        static auto constexpr ORIGIN_SQUARE_MASK        = 0x000000000000FC00;
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
        // no bits:     en passant square after move (calculated from source, target, moved piece)    
        static auto constexpr EN_PASSANT_AFTER_MASK     = 0x0000000000000000;
        // bits 35-41:  draft                           
        static auto constexpr DRAFT_MASK                = 0x000003F800000000;
        // bits 42-43:  
        static auto constexpr TYPE_MASK                 = 0x00000C0000000000;
        // bits 44-63:  score 
        static auto constexpr SCORE_MASK                = 0xFFFFF00000000000; 

        constexpr HashEntry() = default;

        constexpr HashEntry(HashEntry const & other) = default;
        constexpr HashEntry(HashEntry && other) = default;

        HashEntry & operator=(HashEntry const & other) = default;
        HashEntry & operator=(HashEntry && other) = default;

        constexpr HashEntry(HashEntryType const hashEntryType, ZKey const zKey, int const draft, MilliSquare const score)
        : zKey(zKey)
        {
            move |= static_cast<uint_fast64_t>(draft + (1 << 6))<<ffs(DRAFT_MASK);
            move |= static_cast<uint_fast64_t>(hashEntryType) << ffs(TYPE_MASK);
            move |= static_cast<uint_fast64_t>(score + (1 << 19))<<ffs(SCORE_MASK);
        }

        constexpr HashEntry(HashEntryType const hashEntryType,
            ZKey const zKey,
            int const draft,
            MilliSquare const score,
            char const castlingBefore,
            BitBoard const enPassantBefore,
            Square const origin, 
            Square const target,
            Piece const moved, 
            Piece const captured = KING, // KING: no piece captured 
            Piece const promoted = KING, // KING: no piece promoted
            unsigned char const castlingUpdate = 0xFu)
        : zKey(zKey)
        {
            move |= castlingBefore;
            move |= popcount(enPassantBefore) * (ffs(enPassantBefore) << ffs(EN_PASSANT_BEFORE_MASK));
            move |= origin << ffs(ORIGIN_SQUARE_MASK);
            move |= target << ffs(TARGET_SQUARE_MASK);
            move |= moved << ffs(MOVED_PIECE_MASK);
            move |= captured << ffs(CAPTURED_PIECE_MASK);
            move |= promoted << ffs(PROMOTED_PIECE_MASK);
            move |= static_cast<uint_fast64_t>(castlingUpdate) << ffs(CASTLING_UPDATE_MASK);
            move |= static_cast<uint_fast64_t>(draft + (1 << 6)) << ffs(DRAFT_MASK);
            move |= static_cast<uint_fast64_t>(hashEntryType) << ffs(TYPE_MASK);
            move |= static_cast<uint_fast64_t>(score + (1 << 19)) << ffs(SCORE_MASK);
        }

        template<typename result, uint_fast64_t mask>
        auto constexpr value() const
        {
            return static_cast<result>((this->move & mask) >> ffs(mask));
        }

        template<>
        auto constexpr value<BitBoard, EN_PASSANT_BEFORE_MASK>() const
        {
            return SQUARES[(this->move & EN_PASSANT_BEFORE_MASK) >> ffs(EN_PASSANT_BEFORE_MASK)] & ~A1;
        }

        template<>
        auto constexpr value<BitBoard, EN_PASSANT_AFTER_MASK>() const
        {
            if(value<Piece, MOVED_PIECE_MASK>() != PAWN || value<Piece, CAPTURED_PIECE_MASK>() != KING)
            {
                return EMPTY;
            }

            auto const origin = value<Square, ORIGIN_SQUARE_MASK>();
            auto const target = value<Square, TARGET_SQUARE_MASK>();
            return (A1 << ((origin + target) >> 1)) & Files[origin];        
        }

        template<>
        auto constexpr value<int, DRAFT_MASK>() const 
        {
            int const unsignedDraft = static_cast<int>((this->move & DRAFT_MASK) >> ffs(DRAFT_MASK));
            return unsignedDraft - (1 << 6);
        }

        template<>
        auto constexpr value<MilliSquare, SCORE_MASK>() const 
        {
            MilliSquare const unsignedScore = static_cast<MilliSquare>((this->move & 0xFFFFF00000000000) >> ffs(SCORE_MASK));
            return unsignedScore - (1 << 19);
        }

        std::string getUciNotation() const;
        std::string getLongAlgebraicNotation() const;
        std::string getPrintOut() const;

        ZKey zKey = 0;

    private:
        uint_fast64_t move = 0;
    };

    class HashTable
    {
    public:
        HashTable(size_t sizeInMb);

        HashTable(HashTable const & other) = default;
        HashTable(HashTable && other) = default;

        HashTable & operator=(HashTable const & other) = default;
        HashTable & operator=(HashTable && other) = default;

        HashEntry get(ZKey zKey) const;
        bool insert(HashEntry entry);
        
        void clear();

    private:
        std::vector<HashEntry> entries;    
        size_t indexMask;

    };

    class PrincipalVariationTable
    {
    public:
        // standard size: 256K buckets x 4 entries per bucket x 16 bytes per entry = 16MB 
        PrincipalVariationTable(size_t numberOfBuckets = 2 << 18, size_t bucketSize = 4);

        PrincipalVariationTable(PrincipalVariationTable const & other) = default;
        PrincipalVariationTable(PrincipalVariationTable && other) = default;

        HashEntry get(ZKey zKey) const;
        bool insert(HashEntry entry);

        void clear();
   
    private:
        std::vector<HashEntry> entries;
        size_t const indexMask;       
        size_t const bucketSize;
    };
}