#include "HashTable.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace spezi
{
    namespace
    {
        size_t padToPowerOfTwo(size_t const size)
        {
            size_t result = 2;
            while(result < size)
            {
                result *= 2;
            }
            return result;
        }

        char constexpr p[] = " NBRQK";
        char constexpr f[] = "abcdefgh";
        char constexpr r[] = "12345678";
        char constexpr x[] = "xxxxx-";
        char constexpr PR[] = " NBRQ ";
        char constexpr pr[] = " nbrq ";
    }

    std::string HashEntry::getUciNotation() const
    {    
        auto const result = std::string{}
            + f[value<Square, ORIGIN_SQUARE_MASK>() % SquaresPerRank]
            + r[value<Square, ORIGIN_SQUARE_MASK>() / SquaresPerRank]
            + f[value<Square, TARGET_SQUARE_MASK>() % SquaresPerRank]
            + r[value<Square, TARGET_SQUARE_MASK>() / SquaresPerRank]
            + pr[value<Piece, PROMOTED_PIECE_MASK>()];
   
        return result;
    }

    std::string HashEntry::getLongAlgebraicNotation() const
    {    
        auto const result = std::string(1, p[value<Piece, MOVED_PIECE_MASK>()])
            + f[value<Square, ORIGIN_SQUARE_MASK>() % SquaresPerRank]
            + r[value<Square, ORIGIN_SQUARE_MASK>() / SquaresPerRank]
            + x[value<Piece, CAPTURED_PIECE_MASK>()]
            + f[value<Square, TARGET_SQUARE_MASK>() % SquaresPerRank]
            + r[value<Square, TARGET_SQUARE_MASK>() / SquaresPerRank]
            + PR[value<Piece, PROMOTED_PIECE_MASK>()];
    
        if(result == "Ke1-g1 " || result == "Ke8-g8 ")
        {
            return "  0-0  ";
        }
        if(result == "Ke1-c1 " || result == "Ke8-c8 ") 
        {
            return " 0-0-0 ";
        }       
    
        return result;
    }

    std::string HashEntry::getPrintOut() const
    {
        char constexpr type[] = "APC";
        
        std::ostringstream hexRepresentation;
        hexRepresentation<<std::uppercase<<std::hex<<std::setw(16)<<std::setfill('0')<<zKey<<", 0x"<<move;
        auto const epBefore = value<BitBoard, EN_PASSANT_BEFORE_MASK>();
        auto const epBeforeStr = epBefore ? 
            std::string(1, f[ffs(epBefore) % SquaresPerRank]) + r[ffs(epBefore) / SquaresPerRank]
            : "-";
        auto const epAfter = value<BitBoard, EN_PASSANT_AFTER_MASK>();
        auto const epAfterStr = epAfter ? 
            std::string(1, f[ffs(epAfter) % SquaresPerRank]) + r[ffs(epAfter) / SquaresPerRank]
            : "-";
        
        return "0x" + hexRepresentation.str() + "\n"
            + "draft: " + std::to_string(value<int, DRAFT_MASK>())
            + ", score: " + std::to_string(value<MilliSquare, SCORE_MASK>())
            + ", type: " + type[value<HashEntryType, TYPE_MASK>()] + "\n"
            + "move: " + getLongAlgebraicNotation() + "\n"
            + "castling: " + std::to_string(value<unsigned char, CASTLING_BEFORE_MASK>()) 
            + "->" + std::to_string(value<unsigned char, CASTLING_BEFORE_MASK>() & value<unsigned char, CASTLING_UPDATE_MASK>()) + "\n"
            + "e.p.: " + epBeforeStr + "->" + epAfterStr + "\n";
    }

    HashTable::HashTable(size_t const sizeInMb)
    : entries(padToPowerOfTwo(sizeInMb / sizeof(HashEntry))), indexMask(entries.size() - 1)
    {}

    HashEntry HashTable::get(ZKey const zKey) const
    {
        return entries[zKey & indexMask];
    }

    bool HashTable::insert(HashEntry const newEntry)
    {
        auto & oldEntry = entries[newEntry.zKey & indexMask]; 
        
        if(oldEntry.value<HashEntryType, HashEntry::TYPE_MASK>() != PV_NODE
            || oldEntry.value<int, HashEntry::DRAFT_MASK>() <= newEntry.value<int, HashEntry::DRAFT_MASK>())
        {

            oldEntry = newEntry;
            return true;
        }

        return false;
    }

    void HashTable::clear()
    {
        entries.clear();
        entries.resize(indexMask + 1);
    }

    PrincipalVariationTable::PrincipalVariationTable(size_t const numberOfBuckets, size_t const sizeOfBucket)
    : entries(padToPowerOfTwo(numberOfBuckets) * sizeOfBucket), indexMask((entries.size() / sizeOfBucket) - 1), bucketSize(sizeOfBucket)
    {}

    HashEntry PrincipalVariationTable::get(ZKey const zKey) const
    {
        auto maxDraft = -64;
        auto maxDraftIndex = entries.size();

        auto const index = zKey & indexMask;
        for(auto i = index; i < index + bucketSize; ++i)
        {
            auto const entry = entries[i];
            if(entry.zKey == zKey)
            {
                auto const draft = entry.value<int, HashEntry::DRAFT_MASK>();
                if(maxDraft < draft)
                {
                    maxDraftIndex = i;
                    maxDraft = draft;
                }
            }
        }

        return (maxDraftIndex == entries.size()) ? HashEntry {} : entries[maxDraftIndex];
    }

    bool PrincipalVariationTable::insert(HashEntry newEntry)
    {
        auto const index = newEntry.zKey & indexMask;
        for(auto i = index; i < index + bucketSize; ++i)
        {
            auto & entry = entries[i];
            if(entry.zKey == 0
            || (entry.zKey == newEntry.zKey && entry.value<int, HashEntry::DRAFT_MASK>() <= newEntry.value<int, HashEntry::DRAFT_MASK>()))
            {
                entry = newEntry;
                return true;
            }
        }
        return false;
    }

    void PrincipalVariationTable::clear()
    {
        entries.clear();
        entries.resize((indexMask + 1) * bucketSize);
    }
}